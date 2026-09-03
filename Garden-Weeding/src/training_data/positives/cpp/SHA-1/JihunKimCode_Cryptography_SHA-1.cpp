#include <iostream>
#include <vector>
using namespace std;

class SHA1{
	public:
		void padding(string);
		void computation(string);
		int msg_schedule(int, int);
		int function(int, int, int, int);
		unsigned int hash[5];
	private:
		vector<unsigned int> pad_msg;
		unsigned int msg_sch[80];
		unsigned int K[4] = {0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6};
		unsigned int orig_hash[5] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
};

void SHA1::padding(string msg){
	long bits = 8*msg.size();
	size_t count = 0;
	pad_msg.resize(16,0);

	/*Message*/
	for(size_t i = 0; i<msg.size(); i++){
		pad_msg[count/32] = (pad_msg[count/32]<<8) ^ msg[i];
		count += 8;
		if(count > 32*pad_msg.size()-64) pad_msg.resize(pad_msg.size()+16);
	}

	/*1*/
	pad_msg[count/32] = (pad_msg[count/32] << 1) ^ 1;
	count++;
	if(count > 32*pad_msg.size()-64) pad_msg.resize(pad_msg.size()+16);

	/*0's*/
	for(;count<32*pad_msg.size()-64; count++){
		pad_msg[count/32] <<= 1;
		if(count > 32*pad_msg.size()) pad_msg.resize(pad_msg.size()+16);
	}
	
	/*Length*/
	pad_msg[pad_msg.size()-1] = bits;
	pad_msg[pad_msg.size()-2] = (bits>>32);
}

int SHA1::msg_schedule(int i, int t){
	unsigned int x = 0;

	if(t>=0 && t<=15){
		x = pad_msg[i*16+t];
	}
	else if(t>=16&&t<=79){
		x = msg_schedule(i, t-3)^ msg_schedule(i, t-8) ^ msg_schedule(i, t-14) ^ msg_schedule(i, t-16);
		x = (x<<1)|(x>>(32-1));
	}
	return x;
}

int SHA1::function(int t, int x, int y, int z){
	if(t>=0&&t<=19) return (x&y)^(~x&z);
	else if(t>=20&&t<=39) return (x^y^z);
	else if(t>=40&&t<=59) return (x&y)^(x&z)^(y&z);
	else if(t>=60&&t<=79) return (x^y^z);
	return 0;
}

void SHA1::computation(string msg){
	unsigned int a,b,c,d,e,T,con;

	padding(msg);
	for(int i = 0; i<5; i++) hash[i] = orig_hash[i];

	for(size_t i = 0; i<pad_msg.size()/16; i++){
		/*1. Prepare the message schedule*/
		for(int t = 0; t<=79; t++){
			msg_sch[t] = msg_schedule(i, t);
		}
		/*2. Initialize the five working variables*/
		a = hash[0];
		b = hash[1];
		c = hash[2];
		d = hash[3];
		e = hash[4];
		/*3. Calculate five variables*/
		for(int t = 0; t<=79; t++){
			if(t>=0&&t<=19) con = 0;
			else if(t>=20&&t<=39) con = 1;
			else if(t>=40&&t<=59) con = 2;
			else if(t>=60&&t<=79) con = 3;

			T = ((a<<5)|(a>>(32-5)))+function(t,b,c,d)+e+K[con]+msg_sch[t];
			e = d;
			d = c;
			c = ((b<<30)|(b>>(32-30)));
			b = a;
			a = T;
		}
		/*4. Compute*/
		hash[0] += a;
		hash[1] += b;
		hash[2] += c;
		hash[3] += d;
		hash[4] += e;
	}

	for(int i = 0; i<5; i++){
		for(int j = 3; j>=0; j--){
			cout<<hex<<((hash[i]>>(j*8+4))&0x000F);
			cout<<hex<<((hash[i]>>(j*8))&0x000F);
		}
	}
	cout<<'\n';
}

int main(){
	SHA1 s;
	string msg;
	
	for(int i = 0; i<5; i++){
		if(i == 0) msg = "This is a test of SHA-1.";
		else if(i == 1) msg = "Kerckhoff's principle is the foundation on which modern cryptography is built.";
		else if(i == 2) msg = "SHA-1 is no longer considered a secure hashing algorithm.";
		else if(i == 3) msg = "SHA-2 or SHA-3 should be used in place of SHA-1.";
		else if(i == 4) msg = "Never roll your own crypto!";
		s.computation(msg);
	}
	return 0;
}
