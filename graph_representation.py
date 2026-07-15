import os
import sys
import re
import argparse
from clang.cindex import Index, CursorKind

def remove_comments(code: str, lang: str) -> str:
    """
    Removes comments from the code based on the language.
    """
    if lang in ['c', 'cpp']:
        # Remove single-line comments
        code = re.sub(r'//.*', '', code)
        # Remove multi-line comments
        code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
    return code

class CryptoDFGPreprocessor:
    def __init__(self, code: str, lang: str):
        self.code = code
        self.lang = lang
        self.var_map = {}
        self.func_map = {}
        self.var_counter = 0
        self.func_counter = 0
        self.temp_counter = 0
        
        # Standard system/library calls we want to preserve or genericize uniformly
        self.preserved_funcs = {
            'memcpy', 'memset', 'malloc', 'free', 'printf', 'scanf', 'sizeof'
        }

    def get_var_placeholder(self, name: str) -> str:
        if not name:
            return ""
        # If it's a numeric constant, keep it as is
        if re.match(r'^(0x[0-9a-fA-F]+|[0-9]+)$', name):
            return name
        if name not in self.var_map:
            self.var_map[name] = f"v{self.var_counter}"
            self.var_counter += 1
        return self.var_map[name]

    def get_func_placeholder(self, name: str) -> str:
        if not name:
            return ""
        if name in self.preserved_funcs:
            return name
        if name not in self.func_map:
            self.func_map[name] = f"f{self.func_counter}"
            self.func_counter += 1
        return self.func_map[name]

    def get_temp_placeholder(self) -> str:
        name = f"t{self.temp_counter}"
        self.temp_counter += 1
        return name

    def parse_with_clang(self) -> str:
        """
        Parses C/C++ code using Clang AST to extract a simplified Data Flow Graph (DFG).
        """
        args = []
        filename = 'tmp.c'
        if self.lang == 'cpp':
            args = ['-std=c++11']
            filename = 'tmp.cpp'

        try:
            index = Index.create()
            tu = index.parse(filename, args=args, unsaved_files=[(filename, self.code)])
            
            dfg_lines = []
            
            # First pass: map function and variable declarations to ensure consistent renaming
            for cursor in tu.cursor.walk_preorder():
                if cursor.kind == CursorKind.FUNCTION_DECL:
                    name = cursor.spelling
                    if name and name not in self.preserved_funcs:
                        self.get_func_placeholder(name)
                elif cursor.kind in [CursorKind.VAR_DECL, CursorKind.PARM_DECL]:
                    name = cursor.spelling
                    if name:
                        self.get_var_placeholder(name)

            # Second pass: generate flattened DFG statements
            # We traverse the AST and look for operations, assignments, loops, and condition checks
            current_loop_depth = 0

            def traverse(node):
                nonlocal current_loop_depth
                statements = []

                if node.kind in [CursorKind.FOR_STMT, CursorKind.WHILE_STMT, CursorKind.DO_STMT]:
                    current_loop_depth += 1
                    statements.append(f"LOOP_START_DEPTH_{current_loop_depth}")
                    for child in node.get_children():
                        statements.extend(traverse(child))
                    statements.append(f"LOOP_END_DEPTH_{current_loop_depth}")
                    current_loop_depth -= 1
                    return statements

                elif node.kind == CursorKind.IF_STMT:
                    statements.append("BRANCH_IF")
                    for child in node.get_children():
                        statements.extend(traverse(child))
                    return statements

                elif node.kind == CursorKind.BINARY_OPERATOR:
                    # Extract binary operators (assignment, arithmetic, bitwise)
                    children = list(node.get_children())
                    if len(children) == 2:
                        left_rep = get_expr_representation(children[0])
                        right_rep = get_expr_representation(children[1])
                        
                        # Find the operator punctuation token
                        op = "OP"
                        tokens = list(node.get_tokens())
                        for token in tokens:
                            if token.kind.name == 'PUNCTUATION' and token.spelling in ['=', '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=', '<<=', '>>=', '+', '-', '*', '/', '%', '&', '|', '^', '<<', '>>', '==', '!=', '<', '>', '<=', '>=']:
                                op = token.spelling
                                break

                        # Map variables/constants in expression
                        left_val = self.get_var_placeholder(left_rep) if left_rep else "unk"
                        right_val = self.get_var_placeholder(right_rep) if right_rep else "unk"
                        
                        if op in ['=', '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=', '<<=', '>>=']:
                            statements.append(f"{left_val} {op} {right_val}")
                        else:
                            temp = self.get_temp_placeholder()
                            statements.append(f"{temp} = {left_val} {op} {right_val}")
                            # Dynamically store temp variable
                            self.var_map[temp] = temp
                        return statements

                elif node.kind == CursorKind.CALL_EXPR:
                    func_name = node.spelling
                    gen_func = self.get_func_placeholder(func_name)
                    args_rep = []
                    for child in node.get_children():
                        # The first child is often the function reference itself, others are arguments
                        if child.kind != CursorKind.DECL_REF_EXPR:
                            rep = get_expr_representation(child)
                            if rep:
                                args_rep.append(self.get_var_placeholder(rep))
                    statements.append(f"CALL {gen_func}({', '.join(args_rep)})")
                    return statements

                # Recurse for all other nodes that aren't leaf nodes we handled
                for child in node.get_children():
                    statements.extend(traverse(child))
                return statements

            def get_expr_representation(expr_node) -> str:
                """Helper to get a variable name, literal value, or sub-expression name."""
                if expr_node.kind in [CursorKind.DECL_REF_EXPR, CursorKind.MEMBER_REF_EXPR]:
                    return expr_node.spelling
                elif expr_node.kind == CursorKind.INTEGER_LITERAL:
                    # Get literal token value
                    tokens = list(expr_node.get_tokens())
                    if tokens:
                        return tokens[0].spelling
                    return "INT"
                elif expr_node.kind == CursorKind.STRING_LITERAL:
                    return "STR"
                elif expr_node.kind == CursorKind.UNARY_OPERATOR:
                    # E.g. ~x, !x, *x, &x
                    tokens = list(expr_node.get_tokens())
                    op = "~"
                    if tokens:
                        op = tokens[0].spelling
                    child_rep = ""
                    for child in expr_node.get_children():
                        child_rep = get_expr_representation(child)
                    return f"{op}{child_rep}"
                
                # If nested operator, flatten it by traversing and returning a temp placeholder
                sub_statements = traverse(expr_node)
                if sub_statements:
                    dfg_lines.extend(sub_statements)
                    # Return the last assigned variable/temp as the representative of this node
                    last_line = sub_statements[-1]
                    parts = last_line.split()
                    if parts:
                        return parts[0]
                return ""

            # Traverse starting from translation unit cursor
            raw_dfg = traverse(tu.cursor)
            dfg_lines.extend(raw_dfg)

            # Filter and clean output: keep only lines containing cryptographic variables/operations
            cleaned_dfg = []
            for line in dfg_lines:
                # Keep lines with variable assignments, operations, loops, or branch statements
                if line.strip() and not line.startswith("unk"):
                    cleaned_dfg.append(line)

            return "\n".join(cleaned_dfg)

        except Exception as e:
            # Fall back to token-based parser if Clang traversal fails
            return self.fallback_token_parser()

    def fallback_token_parser(self) -> str:
        """
        Robust regex-based fallback to clean, anonymize, and extract data-flow-like statements.
        Useful when Clang library is missing headers or parsing throws errors.
        """
        lines = []
        # Find all words that look like identifiers
        identifiers = set(re.findall(r'\b[a-zA-Z_][a-zA-Z0-9_]*\b', self.code))
        
        # C/C++ keywords to exclude from renaming
        keywords = {
            'int', 'char', 'float', 'double', 'short', 'long', 'void', 'unsigned', 'signed',
            'struct', 'union', 'enum', 'typedef', 'sizeof', 'const', 'static', 'extern',
            'if', 'else', 'for', 'while', 'do', 'switch', 'case', 'default', 'break',
            'continue', 'return', 'goto', 'register', 'volatile', 'inline', 'restrict',
            'true', 'false', 'NULL', 'include', 'define', 'ifdef', 'ifndef', 'endif',
            'memcpy', 'memset', 'malloc', 'free', 'printf', 'scanf'
        }

        user_identifiers = sorted(list(identifiers - keywords))
        
        # Generate stable mappings
        for ident in user_identifiers:
            if ident.isupper() and len(ident) > 4: # Likely a constant macro
                self.var_map[ident] = f"CONST_{ident}"
            else:
                self.var_map[ident] = f"v{self.var_counter}"
                self.var_counter += 1

        # Process line-by-line to extract math, loop, and assignment operations
        loop_depth = 0
        for line in self.code.splitlines():
            line = line.strip()
            if not line:
                continue

            # Track loop blocks approximately
            if any(kw in line for kw in ['for', 'while']) and ('{' in line or line.endswith(')')):
                loop_depth += 1
                lines.append(f"LOOP_START_DEPTH_{loop_depth}")
            elif '}' in line and loop_depth > 0:
                lines.append(f"LOOP_END_DEPTH_{loop_depth}")
                loop_depth -= 1

            # Detect arithmetic/assignment operations or function calls
            if re.search(r'(=|[\+\-\*/%&|^<>]=?|\b\d+\b)', line):
                # Replace user identifiers with placeholders
                words = re.split(r'(\W+)', line)
                newline_parts = []
                for word in words:
                    if word in self.var_map:
                        newline_parts.append(self.var_map[word])
                    else:
                        newline_parts.append(word)
                cleaned_line = "".join(newline_parts).strip()
                # Remove type declarations from the line
                cleaned_line = re.sub(r'\b(int|char|float|double|short|long|unsigned|signed|void|const|static)\s+', '', cleaned_line)
                cleaned_line = re.sub(r'\s*;\s*$', '', cleaned_line) # Strip semicolons
                if cleaned_line:
                    lines.append(cleaned_line)

        return "\n".join(lines)

    def preprocess(self) -> str:
        self.code = remove_comments(self.code, self.lang)
        return self.parse_with_clang()

def get_lang_from_path(path: str) -> str:
    if path.endswith(('.c', '.h')): return 'c'
    if path.endswith(('.cpp', '.hpp', '.hh', '.cc')): return 'cpp'
    return 'undefined'

def standardize_graph_representation(lang, text):
    preprocessor = CryptoDFGPreprocessor(text, lang)
    preprocessed_output = preprocessor.preprocess()
    return preprocessed_output

def main():
    parser = argparse.ArgumentParser(description="Preprocess and generalize a cryptographic code file into an anonymized DFG representation.")
    parser.add_argument("file_path", help="The path to the input cryptographic source file.")
    parser.add_argument("--output", help="Optional path to write the preprocessed output to. Prints to stdout if not specified.")
    args = parser.parse_args()

    if not os.path.exists(args.file_path):
        print(f"Error: File not found at '{args.file_path}'", file=sys.stderr)
        sys.exit(1)

    lang = get_lang_from_path(args.file_path)
    
    with open(args.file_path, 'r', errors='ignore') as f:
        code = f.read()

    preprocessed_output = standardize_graph_representation(lang, code)

    if args.output:
        os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
        with open(args.output, 'w') as f:
            f.write(preprocessed_output)
        print(f"Successfully preprocessed and saved to {args.output}")
    else:
        print(preprocessed_output)

if __name__ == "__main__":
    main()