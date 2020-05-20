#pragma once

namespace pathos {

	enum class InputConstants {
		UNDEFINED,

		// Keyboard
		KEYBOARD_A,
		KEYBOARD_B,
		KEYBOARD_C,
		KEYBOARD_D,
		KEYBOARD_E,
		KEYBOARD_F,
		KEYBOARD_G,
		KEYBOARD_H,
		KEYBOARD_I,
		KEYBOARD_J,
		KEYBOARD_K,
		KEYBOARD_L,
		KEYBOARD_M,
		KEYBOARD_N,
		KEYBOARD_O,
		KEYBOARD_P,
		KEYBOARD_Q,
		KEYBOARD_R,
		KEYBOARD_S,
		KEYBOARD_T,
		KEYBOARD_U,
		KEYBOARD_V,
		KEYBOARD_W,
		KEYBOARD_X,
		KEYBOARD_Y,
		KEYBOARD_Z,
		KEYBOARD_0,
		KEYBOARD_1,
		KEYBOARD_2,
		KEYBOARD_3,
		KEYBOARD_4,
		KEYBOARD_5,
		KEYBOARD_6,
		KEYBOARD_7,
		KEYBOARD_8,
		KEYBOARD_9,
		BACKTICK, // ascii = 0x60

		SHIFT,
		CTRL,
		ALT,

		KEYBOARD_ARROW_LEFT,
		KEYBOARD_ARROW_RIGHT,
		KEYBOARD_ARROW_UP,
		KEYBOARD_ARROW_DOWN,

		// XboxOne Pad
		XBOXONE_A,
		XBOXONE_B,
		XBOXONE_X,
		XBOXONE_Y,
		XBOXONE_DPAD_UP,
		XBOXONE_DPAD_DOWN,
		XBOXONE_DPAD_LEFT,
		XBOXONE_DPAD_RIGHT,
		XBOXONE_START,
		XBOXONE_BACK,
		XBOXONE_LB,
		XBOXONE_LT,
		XBOXONE_RB,
		XBOXONE_RT,
		XBOXONE_L3,
		XBOXONE_R3,
	};

	static constexpr InputConstants asciiToInputConstants[256] = {
		InputConstants::UNDEFINED, // 0
		InputConstants::UNDEFINED, // 1
		InputConstants::UNDEFINED, // 2
		InputConstants::UNDEFINED, // 3
		InputConstants::UNDEFINED, // 4
		InputConstants::UNDEFINED, // 5
		InputConstants::UNDEFINED, // 6
		InputConstants::UNDEFINED, // 7
		InputConstants::UNDEFINED, // 8
		InputConstants::UNDEFINED, // 9
		InputConstants::UNDEFINED, // 10
		InputConstants::UNDEFINED, // 11
		InputConstants::UNDEFINED, // 12
		InputConstants::UNDEFINED, // 13
		InputConstants::UNDEFINED, // 14
		InputConstants::UNDEFINED, // 15
		InputConstants::UNDEFINED, // 16
		InputConstants::UNDEFINED, // 17
		InputConstants::UNDEFINED, // 18
		InputConstants::UNDEFINED, // 19
		InputConstants::UNDEFINED, // 20
		InputConstants::UNDEFINED, // 21
		InputConstants::UNDEFINED, // 22
		InputConstants::UNDEFINED, // 23
		InputConstants::UNDEFINED, // 24
		InputConstants::UNDEFINED, // 25
		InputConstants::UNDEFINED, // 26
		InputConstants::UNDEFINED, // 27
		InputConstants::UNDEFINED, // 28
		InputConstants::UNDEFINED, // 29
		InputConstants::UNDEFINED, // 30
		InputConstants::UNDEFINED, // 31
		InputConstants::UNDEFINED, // 32
		InputConstants::UNDEFINED, // 33
		InputConstants::UNDEFINED, // 34
		InputConstants::UNDEFINED, // 35
		InputConstants::UNDEFINED, // 36
		InputConstants::UNDEFINED, // 37
		InputConstants::UNDEFINED, // 38
		InputConstants::UNDEFINED, // 39
		InputConstants::UNDEFINED, // 40
		InputConstants::UNDEFINED, // 41
		InputConstants::UNDEFINED, // 42
		InputConstants::UNDEFINED, // 43
		InputConstants::UNDEFINED, // 44
		InputConstants::UNDEFINED, // 45
		InputConstants::UNDEFINED, // 46
		InputConstants::UNDEFINED, // 47
		InputConstants::KEYBOARD_0, // 48
		InputConstants::KEYBOARD_1, // 49
		InputConstants::KEYBOARD_2, // 50
		InputConstants::KEYBOARD_3, // 51
		InputConstants::KEYBOARD_4, // 52
		InputConstants::KEYBOARD_5, // 53
		InputConstants::KEYBOARD_6, // 54
		InputConstants::KEYBOARD_7, // 55
		InputConstants::KEYBOARD_8, // 56
		InputConstants::KEYBOARD_9, // 57
		InputConstants::UNDEFINED, // 58
		InputConstants::UNDEFINED, // 59
		InputConstants::UNDEFINED, // 60
		InputConstants::UNDEFINED, // 61
		InputConstants::UNDEFINED, // 62
		InputConstants::UNDEFINED, // 63
		InputConstants::UNDEFINED, // 64
		InputConstants::KEYBOARD_A, // 65
		InputConstants::KEYBOARD_B, // 66
		InputConstants::KEYBOARD_C, // 67
		InputConstants::KEYBOARD_D, // 68
		InputConstants::KEYBOARD_E, // 69
		InputConstants::KEYBOARD_F, // 70
		InputConstants::KEYBOARD_G, // 71
		InputConstants::KEYBOARD_H, // 72
		InputConstants::KEYBOARD_I, // 73
		InputConstants::KEYBOARD_J, // 74
		InputConstants::KEYBOARD_K, // 75
		InputConstants::KEYBOARD_L, // 76
		InputConstants::KEYBOARD_M, // 77
		InputConstants::KEYBOARD_N, // 78
		InputConstants::KEYBOARD_O, // 79
		InputConstants::KEYBOARD_P, // 80
		InputConstants::KEYBOARD_Q, // 81
		InputConstants::KEYBOARD_R, // 82
		InputConstants::KEYBOARD_S, // 83
		InputConstants::KEYBOARD_T, // 84
		InputConstants::KEYBOARD_U, // 85
		InputConstants::KEYBOARD_V, // 86
		InputConstants::KEYBOARD_W, // 87
		InputConstants::KEYBOARD_X, // 88
		InputConstants::KEYBOARD_Y, // 89
		InputConstants::KEYBOARD_Z, // 90
		InputConstants::UNDEFINED, // 91
		InputConstants::UNDEFINED, // 92
		InputConstants::UNDEFINED, // 93
		InputConstants::UNDEFINED, // 94
		InputConstants::UNDEFINED, // 95
		InputConstants::UNDEFINED, // 96
		InputConstants::KEYBOARD_A, // 97
		InputConstants::KEYBOARD_B, // 98
		InputConstants::KEYBOARD_C, // 99
		InputConstants::KEYBOARD_D, // 100
		InputConstants::KEYBOARD_E, // 101
		InputConstants::KEYBOARD_F, // 102
		InputConstants::KEYBOARD_G, // 103
		InputConstants::KEYBOARD_H, // 104
		InputConstants::KEYBOARD_I, // 105
		InputConstants::KEYBOARD_J, // 106
		InputConstants::KEYBOARD_K, // 107
		InputConstants::KEYBOARD_L, // 108
		InputConstants::KEYBOARD_M, // 109
		InputConstants::KEYBOARD_N, // 110
		InputConstants::KEYBOARD_O, // 111
		InputConstants::KEYBOARD_P, // 112
		InputConstants::KEYBOARD_Q, // 113
		InputConstants::KEYBOARD_R, // 114
		InputConstants::KEYBOARD_S, // 115
		InputConstants::KEYBOARD_T, // 116
		InputConstants::KEYBOARD_U, // 117
		InputConstants::KEYBOARD_V, // 118
		InputConstants::KEYBOARD_W, // 119
		InputConstants::KEYBOARD_X, // 120
		InputConstants::KEYBOARD_Y, // 121
		InputConstants::KEYBOARD_Z, // 122
		InputConstants::UNDEFINED, // 123
		InputConstants::UNDEFINED, // 124
		InputConstants::UNDEFINED, // 125
		InputConstants::UNDEFINED, // 126
		InputConstants::UNDEFINED, // 127
		InputConstants::UNDEFINED, // 128
		InputConstants::UNDEFINED, // 129
		InputConstants::UNDEFINED, // 130
		InputConstants::UNDEFINED, // 131
		InputConstants::UNDEFINED, // 132
		InputConstants::UNDEFINED, // 133
		InputConstants::UNDEFINED, // 134
		InputConstants::UNDEFINED, // 135
		InputConstants::UNDEFINED, // 136
		InputConstants::UNDEFINED, // 137
		InputConstants::UNDEFINED, // 138
		InputConstants::UNDEFINED, // 139
		InputConstants::UNDEFINED, // 140
		InputConstants::UNDEFINED, // 141
		InputConstants::UNDEFINED, // 142
		InputConstants::UNDEFINED, // 143
		InputConstants::UNDEFINED, // 144
		InputConstants::UNDEFINED, // 145
		InputConstants::UNDEFINED, // 146
		InputConstants::UNDEFINED, // 147
		InputConstants::UNDEFINED, // 148
		InputConstants::UNDEFINED, // 149
		InputConstants::UNDEFINED, // 150
		InputConstants::UNDEFINED, // 151
		InputConstants::UNDEFINED, // 152
		InputConstants::UNDEFINED, // 153
		InputConstants::UNDEFINED, // 154
		InputConstants::UNDEFINED, // 155
		InputConstants::UNDEFINED, // 156
		InputConstants::UNDEFINED, // 157
		InputConstants::UNDEFINED, // 158
		InputConstants::UNDEFINED, // 159
		InputConstants::UNDEFINED, // 160
		InputConstants::UNDEFINED, // 161
		InputConstants::UNDEFINED, // 162
		InputConstants::UNDEFINED, // 163
		InputConstants::UNDEFINED, // 164
		InputConstants::UNDEFINED, // 165
		InputConstants::UNDEFINED, // 166
		InputConstants::UNDEFINED, // 167
		InputConstants::UNDEFINED, // 168
		InputConstants::UNDEFINED, // 169
		InputConstants::UNDEFINED, // 170
		InputConstants::UNDEFINED, // 171
		InputConstants::UNDEFINED, // 172
		InputConstants::UNDEFINED, // 173
		InputConstants::UNDEFINED, // 174
		InputConstants::UNDEFINED, // 175
		InputConstants::UNDEFINED, // 176
		InputConstants::UNDEFINED, // 177
		InputConstants::UNDEFINED, // 178
		InputConstants::UNDEFINED, // 179
		InputConstants::UNDEFINED, // 180
		InputConstants::UNDEFINED, // 181
		InputConstants::UNDEFINED, // 182
		InputConstants::UNDEFINED, // 183
		InputConstants::UNDEFINED, // 184
		InputConstants::UNDEFINED, // 185
		InputConstants::UNDEFINED, // 186
		InputConstants::UNDEFINED, // 187
		InputConstants::UNDEFINED, // 188
		InputConstants::UNDEFINED, // 189
		InputConstants::UNDEFINED, // 190
		InputConstants::UNDEFINED, // 191
		InputConstants::UNDEFINED, // 192
		InputConstants::UNDEFINED, // 193
		InputConstants::UNDEFINED, // 194
		InputConstants::UNDEFINED, // 195
		InputConstants::UNDEFINED, // 196
		InputConstants::UNDEFINED, // 197
		InputConstants::UNDEFINED, // 198
		InputConstants::UNDEFINED, // 199
		InputConstants::UNDEFINED, // 200
		InputConstants::UNDEFINED, // 201
		InputConstants::UNDEFINED, // 202
		InputConstants::UNDEFINED, // 203
		InputConstants::UNDEFINED, // 204
		InputConstants::UNDEFINED, // 205
		InputConstants::UNDEFINED, // 206
		InputConstants::UNDEFINED, // 207
		InputConstants::UNDEFINED, // 208
		InputConstants::UNDEFINED, // 209
		InputConstants::UNDEFINED, // 210
		InputConstants::UNDEFINED, // 211
		InputConstants::UNDEFINED, // 212
		InputConstants::UNDEFINED, // 213
		InputConstants::UNDEFINED, // 214
		InputConstants::UNDEFINED, // 215
		InputConstants::UNDEFINED, // 216
		InputConstants::UNDEFINED, // 217
		InputConstants::UNDEFINED, // 218
		InputConstants::UNDEFINED, // 219
		InputConstants::UNDEFINED, // 220
		InputConstants::UNDEFINED, // 221
		InputConstants::UNDEFINED, // 222
		InputConstants::UNDEFINED, // 223
		InputConstants::UNDEFINED, // 224
		InputConstants::UNDEFINED, // 225
		InputConstants::UNDEFINED, // 226
		InputConstants::UNDEFINED, // 227
		InputConstants::UNDEFINED, // 228
		InputConstants::UNDEFINED, // 229
		InputConstants::UNDEFINED, // 230
		InputConstants::UNDEFINED, // 231
		InputConstants::UNDEFINED, // 232
		InputConstants::UNDEFINED, // 233
		InputConstants::UNDEFINED, // 234
		InputConstants::UNDEFINED, // 235
		InputConstants::UNDEFINED, // 236
		InputConstants::UNDEFINED, // 237
		InputConstants::UNDEFINED, // 238
		InputConstants::UNDEFINED, // 239
		InputConstants::UNDEFINED, // 240
		InputConstants::UNDEFINED, // 241
		InputConstants::UNDEFINED, // 242
		InputConstants::UNDEFINED, // 243
		InputConstants::UNDEFINED, // 244
		InputConstants::UNDEFINED, // 245
		InputConstants::UNDEFINED, // 246
		InputConstants::UNDEFINED, // 247
		InputConstants::UNDEFINED, // 248
		InputConstants::UNDEFINED, // 249
		InputConstants::UNDEFINED, // 250
		InputConstants::UNDEFINED, // 251
		InputConstants::UNDEFINED, // 252
		InputConstants::UNDEFINED, // 253
		InputConstants::UNDEFINED, // 254
		InputConstants::UNDEFINED, // 255
	};

	constexpr bool isModifierKey(InputConstants key) {
		return key == InputConstants::CTRL || key == InputConstants::SHIFT || key == InputConstants::ALT;
	}

	constexpr bool isArrowKey(InputConstants key) {
		return key == InputConstants::KEYBOARD_ARROW_LEFT || key == InputConstants::KEYBOARD_ARROW_RIGHT
			|| key == InputConstants::KEYBOARD_ARROW_UP || key == InputConstants::KEYBOARD_ARROW_DOWN;
	}

}
