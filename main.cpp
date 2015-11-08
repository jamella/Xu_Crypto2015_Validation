/* This is intended for validating the Appendix C in 
   Zhang B, Xu C, Meier W. Fast Correlation Attacks over Extension Fields, Large-Unit Linear Approximation and Cryptanalysis of SNOW 2.0[M]
     //Advances in Cryptology--CRYPTO 2015. Springer Berlin Heidelberg, 2015: 643-662.
   The NTL library is used for finite field arithmetics.
   It seems they are wrong. Thus, the improvement is not as significant as declared.

   Author: Yuan Yao
   Mail: yaoyuan1216@gmail.com
   Date: 2015-11-08
*/

#include <BerlekampMassey.h>
#include <NTL/GF2EX.h>
#include <NTL/GF2XFactoring.h>
#include <NTL/GF2EXFactoring.h>
#include <boost/lexical_cast.hpp>
#include <array>
#include <iostream>
#include <random>

int main() {
	using boost::lexical_cast;

	auto const
		snow = lexical_cast<NTL::GF2X>("[1 0 0 1 0 1 0 1 1]"),
		aes = lexical_cast<NTL::GF2X>("[1 1 0 1 1 0 0 0 1]");
	BOOST_ASSERT(NTL::IterIrredTest(snow));
	BOOST_ASSERT(NTL::IterIrredTest(aes));
	NTL::GF2E::init(snow);

	NTL::GF2EX snow_extended;// ��AES����Ҳ�ǲ���Լ��
	auto const beta = lexical_cast<NTL::GF2E>("[0 1]");
	SetCoeff(snow_extended, 0, power(beta, 239));
	SetCoeff(snow_extended, 1, power(beta, 48));
	SetCoeff(snow_extended, 2, power(beta, 245));
	SetCoeff(snow_extended, 3, power(beta, 23));
	SetCoeff(snow_extended, 4);
	BOOST_ASSERT(NTL::DetIrredTest(snow_extended));

	size_t constexpr LFSR = 16;
	std::array<NTL::GF2EX, LFSR * 3> State = {};
	SetSeed(NTL::conv<NTL::ZZ>(std::random_device{}()));
	for (size_t i = 0; i != LFSR; ++i) {
		State[i] = NTL::random_GF2EX(deg(snow_extended));
	}

	auto const
		alpha = lexical_cast<NTL::GF2EX>("[[] [1]]"),
		alpha_inverse = InvMod(alpha, snow_extended);
	for (size_t i = LFSR; i != State.size(); ++i) {
		State[i] = (alpha_inverse * State[i - 5] + State[i - 14] + alpha * State[i - 16]) % snow_extended;
	}

	auto const print = [](auto const& f) {
		std::cout << "===================================================\n";
		std::cout << "Coefficients: \n";
		for (auto const& element : f) {
			std::cout << element << "\n";
		}
		std::cout << "Length: " << boost::size(f) << "\n";
		std::cout << "===================================================\n";
	};

	auto const Test = [&State, &print](auto const& M) {
		print(BerlekampMassey(State,
			[&](auto& a, auto const& b) {a = (a + b) % M;},
			[&](auto& a, auto const& b) {a = (a - b) % M;},
			[&](auto const& a, auto const& b) {return MulMod(a, b, M);},
			[&](auto const& a) {return InvMod(a, M);}
		));
	};

	std::cout << "With SNOW\n";
	Test(snow_extended);

	std::cout << "With AES\n";
	NTL::GF2E::init(aes);
	NTL::GF2EX const aes_extended = lexical_cast<NTL::GF2EX>(
		"[[1 1 1 0 0 1 1] [0 0 1 0 0 1 1] [1 0 1 1 0 0 0 1] [0 0 1 1 1 0 1] [0 0 0 1 0 0 1] [1]]"
		);
	BOOST_ASSERT(NTL::DetIrredTest(aes_extended));
	Test(aes_extended);

	return 0;
}