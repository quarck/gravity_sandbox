#pragma once

#include <array>
#include <stdint.h>
#include <numeric>

namespace glText
{
	namespace glFont
	{
		constexpr int RAW_LTR_W = 7;
		constexpr int RAW_LTR_H = 9;
		constexpr int RES_LTR_W = 9;
		constexpr int RES_LTR_H = 13;

#pragma region "font data"
		static const char* _UNKNOWN = 
			"......."
			".?????."
			".?????."
			".?????."
			".?????."
			".?????."
			".?????."
			".?????."
			"......."
			;

		static const char* _SPACECHR =
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			;

		static const char* _A =
			"...X..."
			"...X..."
			"..X.X.."
			"..X.X.."
			".X...X."
			".X...X."
			".XXXXX."
			"X.....X"
			"X.....X"
			;

		static const char* _B =
			"XXXX..."
			"X...X.."
			"X...X.."
			"XXXXX.."
			"X....X."
			"X.....X"
			"X.....X"
			"X....X."
			"XXXXX.."
			;
		static const char* _C =
			".XXXXX."
			"X.....X"
			"X......"
			"X......"
			"X......"
			"X......"
			"X......"
			"X.....X"
			".XXXXX."
			;

		static const char* _D =
			"XXXXD.."
			"X....D."
			"X.....X"
			"X.....X"
			"X.....X"
			"X.....X"
			"X.....D"
			"X....D."
			"XXXXX.."
			;
		static const char* _E =
			"EEEEEEE"
			"E.....E"
			"E......"
			"E......"
			"EEEEE.."
			"E......"
			"E......"
			"E.....E"
			"EEEEEEE"
			;
		static const char* _F =
			"FFFFFFF"
			"F.....F"
			"F......"
			"F......"
			"FFFFF.."
			"F......"
			"F......"
			"F......"
			"F......"
			;
		static const char* _G =
			"..GGGG."
			".G....G"
			"G......"
			"G......"
			"G..GGGG"
			"G..G..G"
			"G.....G"
			".G...G."
			"..GGG.."
			;
		static const char* _H =
			"HH...HH"
			"H.....H"
			"H.....H"
			"H.....H"
			"HHHHHHH"
			"H.....H"
			"H.....H"
			"H.....H"
			"HH...HH"
			;
		static const char* _I =
			"..III.."
			"...I..."
			"...I..."
			"...I..."
			"...I..."
			"...I..."
			"...I..."
			"...I..."
			"..III.."
			;
		static const char* _J =
			"..JJJ.."
			"...J..."
			"...J..."
			"...J..."
			"...J..."
			"...J..."
			"...J..."
			"J..J..."
			".JJ...."
			;
		static const char* _K =
			"KK...KK"
			"K...K.."
			"K..K..."
			"KKK...."
			"K.K...."
			"K..K..."
			"K...k.."
			"K....K."
			"KK...KK"
			;
		static const char* _L =
			"L......"
			"L......"
			"L......"
			"L......"
			"L......"
			"L......"
			"L......"
			"L.....L"
			"LLLLLLL"
			;
		static const char* _M =
			"M.....M"
			"MM...MM"
			"M.M.M.M"
			"M..M..M"
			"M.....M"
			"M.....M"
			"M.....M"
			"M.....M"
			"M.....M"
			;
		static const char* _N =
			"N.....N"
			"NN....N"
			"N.N...N"
			"N.N...N"
			"N..N..N"
			"N...N.N"
			"N...N.N"
			"N....NN"
			"N.....N"
			;
		static const char* _O =
			"..OOO.."
			".O...O."
			"O.....O"
			"O.....O"
			"O.....O"
			"O.....O"
			"O.....O"
			".O...O."
			"..OOO.."
			;
		static const char* _P =
			"PPPPP.."
			"P....P."
			"P.....P"
			"P....P."
			"PPPPP.."
			"P......"
			"P......"
			"P......"
			"PP....."
			;
		static const char* _Q =
			"..OOO.."
			".O...O."
			"O.....O"
			"O.....O"
			"O.....O"
			"O.....O"
			"O....Q."
			".OQQQ.."
			"..OOOQQ"
			;
		static const char* _R =
			"PPPPP.."
			"P....P."
			"P.....P"
			"P....P."
			"PPPPP.."
			"P.R...."
			"P..R..."
			"P...R.."
			"PP...RR"
			;
		static const char* _S =
			".SSS.S."
			"S...SS."
			"S......"
			".S....."
			"..SSS.."
			".....S."
			"......S"
			"SS...S."
			"S.SSS.."
			;
		static const char* _T =
			"TTTTTTT"
			"T..T..T"
			"...T..."
			"...T..."
			"...T..."
			"...T..."
			"...T..."
			"...T..."
			"..TTT.."
			;
		static const char* _U =
			"UU...UU"
			"U.....U"
			"U.....U"
			"U.....U"
			"U.....U"
			"U.....U"
			"U.....U"
			"U.....U"
			".UUUUU."
			;
		static const char* _V =
			"VV...VV"
			"V.....V"
			".V...V."
			".V...V."
			".V...V."
			"..V.V.."
			"..V.V.."
			"...V..."
			"...V..."
			;

		static const char* _W =
			"WW...WW"
			"W.....W"
			"W.....W"
			"W.....W"
			"W.....W"
			"W..W..W"
			"W.W.W.W"
			"WW...WW"
			"W.....W"
			;
		static const char* _X =
			"XX...XX"
			"X.....X"
			".X...X."
			"..X.X.."
			"...X..."
			"..X.X.."
			".X...X."
			"X.....X"
			"XX...XX"
			;
		static const char* _Y =
			"XX...XX"
			"X.....X"
			".X...X."
			"..X.X.."
			"...X..."
			"...Y..."
			"...Y..."
			"...Y..."
			"..YYY.."
			;
		static const char* _Z =
			"ZZZZZZZ"
			"Z.....Z"
			".....Z."
			"....Z.."
			"...Z..."
			"..Z...."
			".Z....."
			"Z.....Z"
			"ZZZZZZZ"
			;
		static const char* _0 =
			"..000.."
			".0...0."
			"0...0.0"
			"0..0..0"
			"0..0..0"
			"0.0...0"
			"0.0...0"
			".0...0."
			"..000.."
			;
		static const char* _1 =
			"...1..."
			"..11..."
			".1.1..."
			"...1..."
			"...1..."
			"...1..."
			"...1..."
			"...1..."
			"..111.."
			;
		static const char* _2 =
			".2222.."
			"2....2."
			".....2."
			"....2.."
			"...2..."
			"..2...."
			".2....."
			"2.....2"
			"2222222"
			;
		static const char* _3 =
			".3333.."
			"3....3."
			"......3"
			".....3."
			"..333.."
			".....3."
			"......3"
			"3....3."
			".3333.."
			;
		static const char* _4 =
			"....444"
			"...4..4"
			"..4...4"
			".4....4"
			"4444444"
			"......4"
			"......4"
			"......4"
			".....44"
			;
		static const char* _5 =
			"5555555"
			"5......"
			"5......"
			"55555.."
			".....5."
			"......5"
			"......5"
			"5....5."
			".5555.."
			;
		static const char* _6 =
			"..6666."
			".6....."
			"6......"
			"66666.."
			"6....6."
			"6.....6"
			"6.....6"
			".6...6."
			"..666.."
			;
		static const char* _7 =
			"7777777"
			"7.....7"
			".....7."
			"....7.."
			"...7..."
			"..7...."
			"..7...."
			"..7...."
			"..7...."
			;
		static const char* _8 =
			"..888.."
			".8...8."
			".8...8."
			"..888.."
			".8...8."
			"8.....8"
			"8.....8"
			".8...8."
			"..888.."
			;
		static const char* _9 =
			"..999.."
			".9...9."
			"9.....9"
			"9.....9"
			".999999"
			"......9"
			"......9"
			".....9."
			".9999.."
			;


		static const char* _Period =
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"...XX.."
			"......."
			;
		static const char* _Comma =
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"...XX.."
			"....X.."
			;
		static const char* _Question =
			".?????."
			"?.....?"
			"......?"
			".....?."
			"....?.."
			"...?..."
			"...?..."
			"......."
			"...?..."
			;
		static const char* _Exlm =
			"...!..."
			"...!..."
			"...!..."
			"...!..."
			"...!..."
			"...!..."
			"...!..."
			"......."
			"...!..."
			;
		static const char* _OpBracket =
			"....(.."
			"...(..."
			"..(...."
			"..(...."
			"..(...."
			"..(...."
			"..(...."
			"...(..."
			"....(.."
			;
		static const char* _ClBracket =
			"..)...."
			"...)..."
			"....).."
			"....).."
			"....).."
			"....).."
			"....).."
			"...)..."
			"..)...."
			;
		static const char* _AT =
			".@@@@@."
			"@.....@"
			"@.....@"
			"@.@@@.@"
			"@.@.@.@"
			"@.@@@@."
			"@......"
			"@.....@"
			".@@@@@."
			;
		static const char* _S_QUOTE =
			"..XX..."
			"...X..."
			"..X...."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			;
		static const char* _D_QUOTE =
			"XX..XX."
			".X...X."
			"X...X.."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			;
		static const char* _DOLLAR =
			"...$..."
			".$$$$$."
			"$..$..."
			"$..$..."
			".$$$$$."
			"...$..$"
			"...$..$"
			".$$$$$."
			"...$..."
			;
		static const char* _PERCENT =
			".%....%"
			"%.%..%."
			".%..%.."
			"....%.."
			"...%..."
			"..%...."
			".%...%."
			".%..%.%"
			"%....%."
			;

		static const char* _HAT =
			"...^..."
			"..^.^.."
			".^...^."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			;
		static const char* _AND =
			"..XX..."
			".X..X.."
			".X..X.."
			".X..X.."
			"..XX..X"
			".X..XX."
			"X...X.."
			"X..X.X."
			".XX...X"
			;
		static const char* _OR =
			"...|..."
			"...|..."
			"...|..."
			"...|..."
			"...|..."
			"...|..."
			"...|..."
			"...|..."
			"...|..."
			;
		static const char* _STAR =
			"......."
			"...X..."
			".X.X.X."
			"..XXX.."
			"XXXXXXX"
			"..XXX.."
			".X.X.X."
			"...X..."
			"......."
			;
		static const char* _MINUS =
			"......."
			"......."
			"......."
			"......."
			"XXXXXXX"
			"......."
			"......."
			"......."
			"......."
			;
		static const char* _PLUS =
			"......."
			"...X..."
			"...X..."
			"...X..."
			"XXXXXXX"
			"...X..."
			"...X..."
			"...X..."
			"......."
			;
		static const char* _LESS =
			"......."
			"....X.."
			"...X..."
			"..X...."
			".X....."
			"..X...."
			"...X..."
			"....X.."
			"......."
			;
		static const char* _GREATER =
			"......."
			"..X...."
			"...X..."
			"....X.."
			".....X."
			"....X.."
			"...X..."
			"..X...."
			"......."
			;
		static const char* _SLASH =
			"......X"
			"......X"
			".....X."
			"....X.."
			"...X..."
			"..X...."
			".X....."
			"X......"
			"X......"
			;
		static const char* _BACKSLASH =
			"X......"
			"X......"
			".X....."
			"..X...."
			"...X..."
			"....X.."
			".....X."
			"......X"
			"......X"
			;

		static const char* _ANGLE_BRACKET_OPEN =
			"..XXX.."
			"..X...."
			"..X...."
			"..X...."
			"..X...."
			"..X...."
			"..X...."
			"..X...."
			"..XXX.."
			;
		static const char* _ANGLE_BRACKET_CLOSE =
			"..XXX.."
			"....X.."
			"....X.."
			"....X.."
			"....X.."
			"....X.."
			"....X.."
			"....X.."
			"..XXX.."
			;





		static const char* _CUR_BRACKET_OPEN =
			"...XX.."
			"..X...."
			"..X...."
			"..X...."
			".X....."
			"..X...."
			"..X...."
			"..X...."
			"...XX.."
			;
		static const char* _CUR_BRACKET_CLOSE =
			"..XX..."
			"....X.."
			"....X.."
			"....X.."
			".....X."
			"....X.."
			"....X.."
			"....X.."
			"..XX..."
			;

		static const char* _TILDA =
			".XX..XX"
			"X..XX.."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			;
		static const char* _HASH =
			"......."
			".X...X."
			"XXXXXXX"
			".X...X."
			".X...X."
			"XXXXXXX"
			".X...X."
			"......."
			"......."
			;
		static const char* _SEMICOL =
			"......."
			"......."
			"..XX..."
			"......."
			"......."
			"......."
			"..XX..."
			"...X..."
			"...X..."
			;

		static const char* _DOUBLE_PERIOD =
			"......."
			"......."
			"..XX..."
			"......."
			"......."
			"......."
			"..XX..."
			"......."
			"......."
			;
		static const char* _UNDERSCORE =
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"......."
			"XXXXXXX"
			;
		static const char* _EUQALS =
			"......."
			"......."
			"......."
			".XXXXX."
			"......."
			".XXXXX."
			"......."
			"......."
			"......."
			;



#pragma endregion

		static const char* letters[256] = { _UNKNOWN };

		template <int H, int W>
		struct FontItem 
		{
			static constexpr int height() noexcept { return H; }
			static constexpr int width() noexcept { return W; }

			std::array<uint8_t, H* W> data;

			uint8_t px(int x, int y) const noexcept { return data[y * W + x];  }
		};

		using FontItemInst = FontItem<RES_LTR_H, RES_LTR_W>;

		constexpr int HALO_FACTOR = 8;
		constexpr int PRELUMINOSITY_FACTOR = 4;
		constexpr int TAIL_LUMINOSITY_FACTOR = 16;
		constexpr int POSTLUMINOSITY_DIV1 = 5;
		constexpr int POSTLUMINOSITY_DIV2 = 10;
		
		void GenerateLetter(FontItemInst& dst, const char* src, int rawH, int rawW) noexcept
		{
			std::vector<unsigned> rawIntensity(dst.data.size(), 0);

			for (int srcY = 0; srcY < rawH; ++srcY)
			{
				for (int srcX = 0; srcX < rawW; ++srcX)
				{
					int val = src[(rawH - srcY - 1) * rawW + srcX] != '.' ? 255 : 0;
					int dstXc = srcX * 1 + 1;
					int dstYc = srcY * 1 + 1;

					// Ignition
					//rawIntensity[(dstYc + 0) * dst.width() + (dstXc - 1)] += val / PRELUMINOSITY_FACTOR;
					//// Full bringhness 
					rawIntensity[(dstYc + 0) * dst.width() + (dstXc + 0)] += val;
					// Post-luminosity 
					rawIntensity[(dstYc + 0) * dst.width() + (dstXc + 1)] += val * POSTLUMINOSITY_DIV1 / POSTLUMINOSITY_DIV2;
					 // tail 
					//rawIntensity[(dstYc + 0) * dst.width() + (dstXc + 2)] += val / TAIL_LUMINOSITY_FACTOR;


					// Upper halo 
	/*				rawIntensity[(dstYc + 1) * dst.width() + (dstXc - 1)] += val / PRELUMINOSITY_FACTOR / HALO_FACTOR;
	*/				rawIntensity[(dstYc + 1) * dst.width() + (dstXc + 0)] += val / HALO_FACTOR;
					rawIntensity[(dstYc + 1) * dst.width() + (dstXc + 1)] += val * POSTLUMINOSITY_DIV1 / POSTLUMINOSITY_DIV2 / HALO_FACTOR;
					//rawIntensity[(dstYc + 1) * dst.width() + (dstXc + 2)] += val / TAIL_LUMINOSITY_FACTOR / HALO_FACTOR;
				}
			}

			for (int idx = 0; idx < dst.data.size(); ++idx)
			{
				unsigned ri = rawIntensity[idx];
				dst.data[idx] = static_cast<uint8_t>(ri <= 255 ? ri : 255);
			}
		}

		static std::vector<FontItemInst> font;
		static bool initialized = false;

		void GenerateFonts() noexcept
		{
			for (int i = 0; i < 256; ++i)
				letters[i] = _UNKNOWN;

			letters[' '] = _SPACECHR;

			letters['A'] = _A;
			letters['B'] = _B;
			letters['C'] = _C;
			letters['D'] = _D;
			letters['E'] = _E;
			letters['F'] = _F;
			letters['G'] = _G;
			letters['H'] = _H;
			letters['I'] = _I;
			letters['J'] = _J;
			letters['K'] = _K;
			letters['L'] = _L;
			letters['M'] = _M;
			letters['N'] = _N;
			letters['O'] = _O;
			letters['P'] = _P;
			letters['Q'] = _Q;
			letters['R'] = _R;
			letters['S'] = _S;
			letters['T'] = _T;
			letters['U'] = _U;
			letters['V'] = _V;
			letters['W'] = _W;
			letters['X'] = _X;
			letters['Y'] = _Y;
			letters['Z'] = _Z;

			letters['a'] = _A;
			letters['b'] = _B;
			letters['c'] = _C;
			letters['d'] = _D;
			letters['e'] = _E;
			letters['f'] = _F;
			letters['g'] = _G;
			letters['h'] = _H;
			letters['i'] = _I;
			letters['j'] = _J;
			letters['k'] = _K;
			letters['l'] = _L;
			letters['m'] = _M;
			letters['n'] = _N;
			letters['o'] = _O;
			letters['p'] = _P;
			letters['q'] = _Q;
			letters['r'] = _R;
			letters['s'] = _S;
			letters['t'] = _T;
			letters['u'] = _U;
			letters['v'] = _V;
			letters['w'] = _W;
			letters['x'] = _X;
			letters['y'] = _Y;
			letters['z'] = _Z;

			letters['0'] = _0;
			letters['1'] = _1;
			letters['2'] = _2;
			letters['3'] = _3;
			letters['4'] = _4;
			letters['5'] = _5;
			letters['6'] = _6;
			letters['7'] = _7;
			letters['8'] = _8;
			letters['9'] = _9;


			letters['.'] = _Period;
			letters[','] = _Comma;
			letters['?'] = _Question;
			letters['!'] = _Exlm;
			letters['('] = _OpBracket;
			letters[')'] = _ClBracket;
			letters['@'] = _AT;
			letters['\''] = _S_QUOTE;
			letters['"'] = _D_QUOTE;
			letters['$'] = _DOLLAR;
			letters['%'] = _PERCENT;
			letters['^'] = _HAT;
			letters['&'] = _AND;
			letters['|'] = _OR;
			letters['*'] = _STAR;
			letters['-'] = _MINUS;
			letters['+'] = _PLUS;
			letters['<'] = _LESS;
			letters['>'] = _GREATER;
			letters['/'] = _SLASH;
			letters['\\'] = _BACKSLASH;
			letters['['] = _ANGLE_BRACKET_OPEN;
			letters[']'] = _ANGLE_BRACKET_CLOSE;

			letters['{'] = _CUR_BRACKET_OPEN;
			letters['}'] = _CUR_BRACKET_CLOSE;
			letters['~'] = _TILDA;
			letters['#'] = _HASH;
			letters[';'] = _SEMICOL;
			letters[':'] = _DOUBLE_PERIOD;
			letters['_'] = _UNDERSCORE;
			letters['='] = _EUQALS;
		


			font.resize(sizeof(letters) / sizeof(letters[0]));
			for (int i = 0; i < font.size(); ++i)
			{
				GenerateLetter(font[i], letters[i], RAW_LTR_H, RAW_LTR_W);
			}
		}

		FontItemInst GetFontItem(unsigned char ltr) noexcept
		{
			if (!initialized)
			{
				GenerateFonts();
				initialized = true;
			}

			return font[ltr];
		}

	}

	struct Label
	{
		int width;
		int height;
		std::vector<uint32_t> data;

		Label(uint32_t bgColor, uint32_t fgColor, const std::string& text)
			: width(0)
			, height(0) 
		{
			Update(text, bgColor, fgColor);
		}

		Label(uint32_t bgColor,
			std::initializer_list<std::pair<uint32_t, std::string>> texts)
			: width(0)
			, height(0)
		{
			Update(bgColor, texts);
		}

		uint32_t& px(int x, int y) noexcept { return data[y * width + x]; }
		const uint32_t& px(int x, int y) const noexcept { return data[y * width + x]; }

		void Update(
			uint32_t bgColor,
			std::initializer_list<std::pair<uint32_t, std::string>> texts
		) noexcept
		{
			size_t maxLen = std::reduce(texts.begin(), texts.end(),
				static_cast<size_t>(0), // init 
				[&](const size_t& mx, const std::pair<uint32_t, std::string> & s) { return mx > s.second.size() ? mx : s.second.size(); });

			const int imgW = (int)(maxLen * glFont::RES_LTR_W);
			const int imgH = (int)(texts.size() * glFont::RES_LTR_H); // always one-liners atm

			if (width != imgW || height != imgH)
			{
				width = imgW;
				height = imgH;
				data.resize(width * height);
			}

			int vIdx = 0;
			for (const auto& textP: texts)
			{
				const auto& text = textP.second;
				const auto fgColor = textP.first;

				uint32_t fgR = fgColor & 0xff;
				uint32_t fgG = (fgColor >> 8) & 0xff;
				uint32_t fgB = (fgColor >> 16) & 0xff;

				for (int i = 0; i < maxLen; ++i)
				{
					int dst_pos = i * glFont::RES_LTR_W + glFont::RES_LTR_H * width * ((int)texts.size() - vIdx - 1);
					int src_pos = 0;

					const auto& fi = i < text.size() ? glFont::GetFontItem((unsigned char)text[i]) : glFont::GetFontItem(' ');

					for (int y = 0; y < glFont::RES_LTR_H; ++y)
					{
						for (int x = 0; x < glFont::RES_LTR_W; ++x)
						{
							auto val = fi.data[src_pos++];

							if (val == 0) // bg
								data[dst_pos++] = bgColor;
							else
							{
								uint32_t r = fgR * val / 255;
								uint32_t g = fgG * val / 255;
								uint32_t b = fgB * val / 255;

								data[dst_pos++] = 0xff000000 | (b << 16) | (g << 8) | r;
							}
						}

						dst_pos += width - glFont::RES_LTR_W;
					}
				} // for (int i...

				++vIdx;
			}
		}

		void Update(
			const std::string& text,
			uint32_t bgColor,
			uint32_t fgColor
		) noexcept
		{
			const int imgW = (int)(text.size() * glFont::RES_LTR_W);
			const int imgH = (int)(glFont::RES_LTR_H); // always one-liners atm

			if (width != imgW || height != imgH)
			{
				width = imgW;
				height = imgH;
				data.resize(width * height);
			}

			uint32_t fgR = fgColor & 0xff;
			uint32_t fgG = (fgColor >> 8) & 0xff;
			uint32_t fgB = (fgColor >> 16) & 0xff;

			for (int i = 0; i < text.size(); ++i)
			{
				int dst_pos = i * glFont::RES_LTR_W;
				int src_pos = 0;

				const auto& fi = glFont::GetFontItem((unsigned char)text[i]);

				for (int y = 0; y < glFont::RES_LTR_H; ++y)
				{
					for (int x = 0; x < glFont::RES_LTR_W; ++x)
					{
						auto val = fi.data[src_pos++];

						if (val == 0) // bg
							data[dst_pos++] = bgColor;
						else
						{
							uint32_t r = fgR * val / 255;
							uint32_t g = fgG * val / 255;
							uint32_t b = fgB * val / 255;

							data[dst_pos++] = 0xff000000 | (b << 16) | (g << 8) | r;
						}
					}

					dst_pos += width - glFont::RES_LTR_W;
				}
			} // for (int i...
		}

		void DrawAt(float x, float y)
		{
			glRasterPos2f(x, y);
			glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
		}
	};
}