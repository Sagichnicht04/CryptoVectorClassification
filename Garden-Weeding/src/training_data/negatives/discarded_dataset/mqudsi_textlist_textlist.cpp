#include <string>
#include <deque>
#include <sstream>
#include <iostream>

namespace TextList
{
    #define ENUM_FLAG_OPERATOR(T,X) inline T operator X (T lhs, T rhs) { return (T) (static_cast<std::underlying_type_t <T>>(lhs) X static_cast<std::underlying_type_t <T>>(rhs)); } 
    #define ENUM_FLAGS(T) \
    enum class T; \
    inline T operator ~ (T t) { return (T) (~static_cast<std::underlying_type_t <T>>(t)); } \
    ENUM_FLAG_OPERATOR(T,|) \
    ENUM_FLAG_OPERATOR(T,^) \
    ENUM_FLAG_OPERATOR(T,&) \
    enum class T

    ENUM_FLAGS(Formatting)
    {
        OxfordSeparator = 1,
        Spaces = 2,
        None = 0,
        Default = Formatting::OxfordSeparator | Formatting::Spaces,
    };

    ENUM_FLAGS(Fish)
    {
        OneFish = 0,
        TwoFish = 1,
        RedFish = 2,
        BlueFish = 4
    };

    ENUM_FLAGS(Hands)
    {
        NoHands = 0,
        OneHand = 1 << 0,
        TwoHands = 1 << 1,
        LeftHand = 1 << 2,
        RightHand = 1 << 3
    };

    template<class T>
    static std::string Make(const T& entities, Formatting formatting = Formatting::Default, std::string conjunction = "and", std::string separator = ",")
    {
        bool oxfordComma = ((formatting & Formatting::OxfordSeparator) == Formatting::OxfordSeparator) || conjunction.empty();
        std::string space = ((formatting & Formatting::Spaces) == Formatting::Spaces) ? " " : "";

        std::stringstream sb;
        auto buffer = std::deque<typename T::value_type>();

        for (const auto &e : entities)
        {
            buffer.push_back(e);
            if (buffer.size() < 4)
            {
                continue;
            }

            //three elements have been guaranteed put aside
            auto t = buffer.front();
            buffer.pop_front();
            sb << t << separator << space;
        }

        if (buffer.size() >= 1)
        {
            sb << buffer.front();
            buffer.pop_front();
        }

        //remember, buffer count has dropped
        if (buffer.size() == 1) //means guaranteed only ever two entries
        {
            if (conjunction.empty())
            {
                sb << separator << space << buffer.front();
                buffer.pop_front();
            }
            else
            {
                sb << space << conjunction << space << buffer.front();
                buffer.pop_front();
            }
        }
        else if (buffer.size() == 2)
        {
            sb << separator << space << buffer.front() << (oxfordComma ? separator.c_str() : "") << space;
            buffer.pop_front();

            if (conjunction.empty())
            {
                sb << buffer.front();
                buffer.pop_front();
            }
            else
            {
                sb << conjunction << space << buffer.front();
                buffer.pop_front();
            }       
        }
        return sb.str();
    }
}

using namespace TextList;

#include <vector>
int main(int argc, const char *argv[])
{
    uint32_t failCount = 0;
    auto allNames = std::vector<std::string> { "Meg", "Jo", "Beth", "Amy" };
    auto names = std::vector<std::string>();
    names.reserve(allNames.size());

    Hands hands = Hands::OneHand | Hands::TwoHands;
    if ( ( (hands & ~Hands::OneHand) ^ (Hands::TwoHands) ) == Hands::NoHands)
    {
        std::cout << "Look ma, no hands!" << std::endl;
    }

    bool oxford = true;
    bool spaces = true;
    bool conjunction = true;

    auto with = [&] (bool x) {
        return x ? "" : "out";
    };

    {
        auto textList = (TextList::Make(names));
        std::cout << ("***Empty list***");
        std::cout << textList.c_str();
    }

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            for (int k = 0; k < 2; ++k)
            {
                printf("***With%s oxford comma, with%s spaces, with%s conjunction***\n", with(oxford), with(spaces), with(conjunction));

                names.clear();
                for (auto name : allNames)
                {
                    names.push_back(name);
                    TextList::Formatting flags = (TextList::Formatting) ((spaces ? TextList::Formatting::Spaces : TextList::Formatting::None) |
                                (oxford ? TextList::Formatting::OxfordSeparator : TextList::Formatting::None))  ;
                    auto textList = TextList::Make(names, flags, (conjunction ? "and" : ""));
                    printf(" %s\n", textList.c_str());
                }
                printf("\n");
                conjunction = !conjunction;
            }
            spaces = !spaces;
        }
        oxford = !oxford;
    }

    return true;
}
