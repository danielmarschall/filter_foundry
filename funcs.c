/*
	This file is part of "Filter Foundry", a filter plugin for Adobe Photoshop
	Copyright (C) 2003-2009 Toby Thain, toby@telegraphics.net
	Copyright (C) 2018-2024 Daniel Marschall, ViaThinkSoft

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef MAC_ENV
#include <fp.h>
#endif

#include <math.h>
#include <stdlib.h>

#ifndef PARSERTEST
#include "ff.h"
#else
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define int32_t int
#endif

#include "funcs.h"
#include "y.tab.h"

#include "node.h" // for symbol "var[]"

#define RINT //no rounding for now

//#if TARGET_API_MAC_CARBON
// this is another incompatibility between Classic stdclib and OS X stdclib
// ***FIXME: need to access real OS X includes for Carbon build
//#undef RAND_MAX
//#define RAND_MAX    0x7fffffff
//#endif

const int FACTORY_COS_LOOKUP[1024] = {
	16384, 16383, 16382, 16381, 16379, 16376, 16372, 16368, 16364, 16359, 16353, 16346, 16339, 16331, 16323,
	16314, 16305, 16294, 16284, 16272, 16260, 16248, 16234, 16221, 16206, 16191, 16175, 16159, 16142, 16125,
	16107, 16088, 16069, 16049, 16028, 16007, 15985, 15963, 15940, 15917, 15893, 15868, 15842, 15817, 15790,
	15763, 15735, 15707, 15678, 15649, 15618, 15588, 15557, 15525, 15492, 15459, 15426, 15392, 15357, 15322,
	15286, 15249, 15212, 15175, 15136, 15098, 15058, 15018, 14978, 14937, 14895, 14853, 14811, 14767, 14723,
	14679, 14634, 14589, 14543, 14496, 14449, 14401, 14353, 14304, 14255, 14205, 14155, 14104, 14053, 14001,
	13948, 13895, 13842, 13788, 13733, 13678, 13622, 13566, 13510, 13452, 13395, 13337, 13278, 13219, 13159,
	13099, 13039, 12977, 12916, 12854, 12791, 12728, 12665, 12601, 12536, 12471, 12406, 12340, 12273, 12207,
	12139, 12072, 12003, 11935, 11866, 11796, 11726, 11656, 11585, 11513, 11442, 11370, 11297, 11224, 11151,
	11077, 11003, 10928, 10853, 10777, 10701, 10625, 10548, 10471, 10394, 10316, 10237, 10159, 10080, 10000,
	9920, 9840, 9760, 9679, 9597, 9516, 9434, 9351, 9269, 9186, 9102, 9018, 8934, 8850, 8765, 8680, 8595,
	8509, 8423, 8336, 8250, 8163, 8075, 7988, 7900, 7812, 7723, 7634, 7545, 7456, 7366, 7276, 7186, 7096,
	7005, 6914, 6822, 6731, 6639, 6547, 6455, 6362, 6270, 6176, 6083, 5990, 5896, 5802, 5708, 5614, 5519,
	5424, 5329, 5234, 5139, 5043, 4948, 4852, 4756, 4659, 4563, 4466, 4369, 4273, 4175, 4078, 3981, 3883,
	3785, 3687, 3589, 3491, 3393, 3295, 3196, 3097, 2998, 2900, 2801, 2702, 2602, 2503, 2404, 2304, 2205,
	2105, 2005, 1905, 1806, 1706, 1606, 1505, 1405, 1305, 1205, 1105, 1004, 904, 804, 703, 603, 502, 402,
	301, 201, 100, -100, -201, -301, -402, -502, -603, -703, -804, -904, -1004, -1105, -1205, -1305, -1405,
	-1505, -1606, -1706, -1806, -1905, -2005, -2105, -2205, -2304, -2404, -2503, -2602, -2702, -2801, -2900,
	-2998, -3097, -3196, -3295, -3393, -3491, -3589, -3687, -3785, -3883, -3981, -4078, -4175, -4273, -4369,
	-4466, -4563, -4659, -4756, -4852, -4948, -5043, -5139, -5234, -5329, -5424, -5519, -5614, -5708, -5802,
	-5896, -5990, -6083, -6176, -6270, -6362, -6455, -6547, -6639, -6731, -6822, -6914, -7005, -7096, -7186,
	-7276, -7366, -7456, -7545, -7634, -7723, -7812, -7900, -7988, -8075, -8163, -8250, -8336, -8423, -8509,
	-8595, -8680, -8765, -8850, -8934, -9018, -9102, -9186, -9269, -9351, -9434, -9516, -9597, -9679, -9760,
	-9840, -9920, -10000, -10080, -10159, -10237, -10316, -10394, -10471, -10548, -10625, -10701, -10777,
	-10853, -10928, -11003, -11077, -11151, -11224, -11297, -11370, -11442, -11513, -11585, -11656, -11726,
	-11796, -11866, -11935, -12003, -12072, -12139, -12207, -12273, -12340, -12406, -12471, -12536, -12601,
	-12665, -12728, -12791, -12854, -12916, -12977, -13039, -13099, -13159, -13219, -13278, -13337, -13395,
	-13452, -13510, -13566, -13622, -13678, -13733, -13788, -13842, -13895, -13948, -14001, -14053, -14104,
	-14155, -14205, -14255, -14304, -14353, -14401, -14449, -14496, -14543, -14589, -14634, -14679, -14723,
	-14767, -14811, -14853, -14895, -14937, -14978, -15018, -15058, -15098, -15136, -15175, -15212, -15249,
	-15286, -15322, -15357, -15392, -15426, -15459, -15492, -15525, -15557, -15588, -15618, -15649, -15678,
	-15707, -15735, -15763, -15790, -15817, -15842, -15868, -15893, -15917, -15940, -15963, -15985, -16007,
	-16028, -16049, -16069, -16088, -16107, -16125, -16142, -16159, -16175, -16191, -16206, -16221, -16234,
	-16248, -16260, -16272, -16284, -16294, -16305, -16314, -16323, -16331, -16339, -16346, -16353, -16359,
	-16364, -16368, -16372, -16376, -16379, -16381, -16382, -16383, -16384, -16384, -16383, -16382, -16381,
	-16379, -16376, -16372, -16368, -16364, -16359, -16353, -16346, -16339, -16331, -16323, -16314, -16305,
	-16294, -16284, -16272, -16260, -16248, -16234, -16221, -16206, -16191, -16175, -16159, -16142, -16125,
	-16107, -16088, -16069, -16049, -16028, -16007, -15985, -15963, -15940, -15917, -15893, -15868, -15842,
	-15817, -15790, -15763, -15735, -15707, -15678, -15649, -15618, -15588, -15557, -15525, -15492, -15459,
	-15426, -15392, -15357, -15322, -15286, -15249, -15212, -15175, -15136, -15098, -15058, -15018, -14978,
	-14937, -14895, -14853, -14811, -14767, -14723, -14679, -14634, -14589, -14543, -14496, -14449, -14401,
	-14353, -14304, -14255, -14205, -14155, -14104, -14053, -14001, -13948, -13895, -13842, -13788, -13733,
	-13678, -13622, -13566, -13510, -13452, -13395, -13337, -13278, -13219, -13159, -13099, -13039, -12977,
	-12916, -12854, -12791, -12728, -12665, -12601, -12536, -12471, -12406, -12340, -12273, -12207, -12139,
	-12072, -12003, -11935, -11866, -11796, -11726, -11656, -11585, -11513, -11442, -11370, -11297, -11224,
	-11151, -11077, -11003, -10928, -10853, -10777, -10701, -10625, -10548, -10471, -10394, -10316, -10237,
	-10159, -10080, -10000, -9920, -9840, -9760, -9679, -9597, -9516, -9434, -9351, -9269, -9186, -9102,
	-9018, -8934, -8850, -8765, -8680, -8595, -8509, -8423, -8336, -8250, -8163, -8075, -7988, -7900, -7812,
	-7723, -7634, -7545, -7456, -7366, -7276, -7186, -7096, -7005, -6914, -6822, -6731, -6639, -6547, -6455,
	-6362, -6270, -6176, -6083, -5990, -5896, -5802, -5708, -5614, -5519, -5424, -5329, -5234, -5139, -5043,
	-4948, -4852, -4756, -4659, -4563, -4466, -4369, -4273, -4175, -4078, -3981, -3883, -3785, -3687, -3589,
	-3491, -3393, -3295, -3196, -3097, -2998, -2900, -2801, -2702, -2602, -2503, -2404, -2304, -2205, -2105,
	-2005, -1905, -1806, -1706, -1606, -1505, -1405, -1305, -1205, -1105, -1004, -904, -804, -703, -603, -502,
	-402, -301, -201, -100, 100, 201, 301, 402, 502, 603, 703, 804, 904, 1004, 1105, 1205, 1305, 1405, 1505,
	1606, 1706, 1806, 1905, 2005, 2105, 2205, 2304, 2404, 2503, 2602, 2702, 2801, 2900, 2998, 3097, 3196,
	3295, 3393, 3491, 3589, 3687, 3785, 3883, 3981, 4078, 4175, 4273, 4369, 4466, 4563, 4659, 4756, 4852,
	4948, 5043, 5139, 5234, 5329, 5424, 5519, 5614, 5708, 5802, 5896, 5990, 6083, 6176, 6270, 6362, 6455,
	6547, 6639, 6731, 6822, 6914, 7005, 7096, 7186, 7276, 7366, 7456, 7545, 7634, 7723, 7812, 7900, 7988,
	8075, 8163, 8250, 8336, 8423, 8509, 8595, 8680, 8765, 8850, 8934, 9018, 9102, 9186, 9269, 9351, 9434,
	9516, 9597, 9679, 9760, 9840, 9920, 10000, 10080, 10159, 10237, 10316, 10394, 10471, 10548, 10625, 10701,
	10777, 10853, 10928, 11003, 11077, 11151, 11224, 11297, 11370, 11442, 11513, 11585, 11656, 11726, 11796,
	11866, 11935, 12003, 12072, 12139, 12207, 12273, 12340, 12406, 12471, 12536, 12601, 12665, 12728, 12791,
	12854, 12916, 12977, 13039, 13099, 13159, 13219, 13278, 13337, 13395, 13452, 13510, 13566, 13622, 13678,
	13733, 13788, 13842, 13895, 13948, 14001, 14053, 14104, 14155, 14205, 14255, 14304, 14353, 14401, 14449,
	14496, 14543, 14589, 14634, 14679, 14723, 14767, 14811, 14853, 14895, 14937, 14978, 15018, 15058, 15098,
	15136, 15175, 15212, 15249, 15286, 15322, 15357, 15392, 15426, 15459, 15492, 15525, 15557, 15588, 15618,
	15649, 15678, 15707, 15735, 15763, 15790, 15817, 15842, 15868, 15893, 15917, 15940, 15963, 15985, 16007,
	16028, 16049, 16069, 16088, 16107, 16125, 16142, 16159, 16175, 16191, 16206, 16221, 16234, 16248, 16260,
	16272, 16284, 16294, 16305, 16314, 16323, 16331, 16339, 16346, 16353, 16359, 16364, 16368, 16372, 16376,
	16379, 16381, 16382, 16383, 16384 };

const uint16_t FACTORY_C2D_LOOKUP[1024] = {
	64, 128, 191, 255, 319, 383, 447, 511, 575, 639, 703, 767, 831, 895, 959, 1023, 1087, 1151, 1215, 1279,
	1343, 1407, 1471, 1535, 1599, 1663, 1727, 1791, 1855, 1919, 1983, 2047, 2111, 2175, 2239, 2303, 2366,
	2430, 2494, 2558, 2622, 2686, 2750, 2814, 2878, 2942, 3005, 3069, 3133, 3197, 3261, 3325, 3388, 3452,
	3516, 3580, 3644, 3708, 3771, 3835, 3899, 3963, 4026, 4090, 4154, 4218, 4281, 4345, 4409, 4473, 4536,
	4600, 4664, 4727, 4791, 4855, 4918, 4982, 5046, 5109, 5173, 5236, 5300, 5364, 5427, 5491, 5554, 5618,
	5681, 5745, 5808, 5872, 5935, 5999, 6062, 6126, 6189, 6252, 6316, 6379, 6443, 6506, 6569, 6633, 6696,
	6759, 6823, 6886, 6949, 7013, 7076, 7139, 7202, 7266, 7329, 7392, 7455, 7518, 7582, 7645, 7708, 7771,
	7834, 7897, 7960, 8023, 8086, 8149, 8212, 8275, 8338, 8401, 8464, 8527, 8590, 8653, 8716, 8779, 8841,
	8904, 8967, 9030, 9093, 9155, 9218, 9281, 9344, 9406, 9469, 9532, 9594, 9657, 9720, 9782, 9845, 9907,
	9970, 10032, 10095, 10157, 10220, 10282, 10345, 10407, 10470, 10532, 10594, 10657, 10719, 10781, 10843,
	10906, 10968, 11030, 11092, 11155, 11217, 11279, 11341, 11403, 11465, 11527, 11589, 11651, 11713, 11775,
	11837, 11899, 11961, 12023, 12085, 12146, 12208, 12270, 12332, 12394, 12455, 12517, 12579, 12640, 12702,
	12764, 12825, 12887, 12948, 13010, 13071, 13133, 13194, 13256, 13317, 13379, 13440, 13501, 13563, 13624,
	13685, 13746, 13808, 13869, 13930, 13991, 14052, 14113, 14174, 14235, 14296, 14357, 14418, 14479, 14540,
	14601, 14662, 14723, 14784, 14844, 14905, 14966, 15027, 15087, 15148, 15208, 15269, 15330, 15390, 15451,
	15511, 15572, 15632, 15693, 15753, 15813, 15874, 15934, 15994, 16054, 16115, 16175, 16235, 16295, 16355,
	16415, 16475, 16535, 16595, 16655, 16715, 16775, 16835, 16895, 16955, 17015, 17074, 17134, 17194, 17254,
	17313, 17373, 17432, 17492, 17551, 17611, 17670, 17730, 17789, 17849, 17908, 17967, 18027, 18086, 18145,
	18204, 18264, 18323, 18382, 18441, 18500, 18559, 18618, 18677, 18736, 18795, 18854, 18912, 18971, 19030,
	19089, 19147, 19206, 19265, 19323, 19382, 19440, 19499, 19557, 19616, 19674, 19733, 19791, 19849, 19908,
	19966, 20024, 20082, 20141, 20199, 20257, 20315, 20373, 20431, 20489, 20547, 20605, 20663, 20720, 20778,
	20836, 20894, 20951, 21009, 21067, 21124, 21182, 21239, 21297, 21354, 21412, 21469, 21526, 21584, 21641,
	21698, 21756, 21813, 21870, 21927, 21984, 22041, 22098, 22155, 22212, 22269, 22326, 22383, 22439, 22496,
	22553, 22610, 22666, 22723, 22780, 22836, 22893, 22949, 23006, 23062, 23118, 23175, 23231, 23287, 23344,
	23400, 23456, 23512, 23568, 23624, 23680, 23736, 23792, 23848, 23904, 23960, 24016, 24071, 24127, 24183,
	24238, 24294, 24350, 24405, 24461, 24516, 24572, 24627, 24682, 24738, 24793, 24848, 24904, 24959, 25014,
	25069, 25124, 25179, 25234, 25289, 25344, 25399, 25454, 25509, 25563, 25618, 25673, 25727, 25782, 25836,
	25891, 25945, 26000, 26054, 26109, 26163, 26217, 26271, 26326, 26380, 26434, 26488, 26542, 26596, 26650,
	26704, 26758, 26812, 26866, 26920, 26974, 27027, 27081, 27135, 27188, 27242, 27295, 27349, 27402, 27456,
	27509, 27563, 27616, 27669, 27722, 27776, 27829, 27882, 27935, 27988, 28041, 28094, 28147, 28200, 28253,
	28305, 28358, 28411, 28464, 28516, 28569, 28621, 28674, 28726, 28779, 28831, 28884, 28936, 28988, 29041,
	29093, 29145, 29197, 29249, 29301, 29353, 29405, 29457, 29509, 29561, 29613, 29664, 29716, 29768, 29820,
	29871, 29923, 29974, 30026, 30077, 30129, 30180, 30231, 30283, 30334, 30385, 30436, 30488, 30539, 30590,
	30641, 30692, 30743, 30794, 30844, 30895, 30946, 30997, 31047, 31098, 31149, 31199, 31250, 31300, 31351,
	31401, 31452, 31502, 31552, 31602, 31653, 31703, 31753, 31803, 31853, 31903, 31953, 32003, 32053, 32103,
	32153, 32202, 32252, 32302, 32351, 32401, 32451, 32500, 32550, 32599, 32649, 32698, 32747, 32796, 32846,
	32895, 32944, 32993, 33042, 33091, 33140, 33189, 33238, 33287, 33336, 33385, 33434, 33482, 33531, 33580,
	33628, 33677, 33725, 33774, 33822, 33871, 33919, 33967, 34015, 34064, 34112, 34160, 34208, 34256, 34304,
	34352, 34400, 34448, 34496, 34544, 34592, 34639, 34687, 34735, 34782, 34830, 34877, 34925, 34972, 35020,
	35067, 35115, 35162, 35209, 35256, 35303, 35351, 35398, 35445, 35492, 35539, 35586, 35633, 35679, 35726,
	35773, 35820, 35866, 35913, 35960, 36006, 36053, 36099, 36146, 36192, 36239, 36285, 36331, 36377, 36424,
	36470, 36516, 36562, 36608, 36654, 36700, 36746, 36792, 36838, 36883, 36929, 36975, 37021, 37066, 37112,
	37157, 37203, 37248, 37294, 37339, 37385, 37430, 37475, 37520, 37566, 37611, 37656, 37701, 37746, 37791,
	37836, 37881, 37926, 37971, 38015, 38060, 38105, 38149, 38194, 38239, 38283, 38328, 38372, 38417, 38461,
	38505, 38550, 38594, 38638, 38682, 38727, 38771, 38815, 38859, 38903, 38947, 38991, 39035, 39078, 39122,
	39166, 39210, 39253, 39297, 39341, 39384, 39428, 39471, 39515, 39558, 39601, 39645, 39688, 39731, 39774,
	39818, 39861, 39904, 39947, 39990, 40033, 40076, 40119, 40161, 40204, 40247, 40290, 40332, 40375, 40418,
	40460, 40503, 40545, 40588, 40630, 40673, 40715, 40757, 40799, 40842, 40884, 40926, 40968, 41010, 41052,
	41094, 41136, 41178, 41220, 41262, 41303, 41345, 41387, 41429, 41470, 41512, 41553, 41595, 41636, 41678,
	41719, 41761, 41802, 41843, 41885, 41926, 41967, 42008, 42049, 42090, 42131, 42172, 42213, 42254, 42295,
	42336, 42376, 42417, 42458, 42499, 42539, 42580, 42620, 42661, 42701, 42742, 42782, 42823, 42863, 42903,
	42944, 42984, 43024, 43064, 43104, 43144, 43184, 43224, 43264, 43304, 43344, 43384, 43424, 43463, 43503,
	43543, 43582, 43622, 43662, 43701, 43741, 43780, 43820, 43859, 43898, 43938, 43977, 44016, 44055, 44094,
	44134, 44173, 44212, 44251, 44290, 44329, 44368, 44406, 44445, 44484, 44523, 44562, 44600, 44639, 44677,
	44716, 44755, 44793, 44832, 44870, 44908, 44947, 44985, 45023, 45062, 45100, 45138, 45176, 45214, 45252,
	45290, 45328, 45366, 45404, 45442, 45480, 45518, 45555, 45593, 45631, 45668, 45706, 45744, 45781, 45819,
	45856, 45894, 45931, 45968, 46006, 46043, 46080, 46117, 46155, 46192, 46229, 46266, 46303, 46340, 46377,
	46414, 46451, 46488, 46525, 46561, 46598, 46635, 46672, 46708, 46745, 46781, 46818, 46854, 46891, 46927,
	46964, 47000, 47037, 47073, 47109, 47145, 47182, 47218, 47254, 47290, 47326, 47362, 47398, 47434, 47470,
	47506, 47542, 47577, 47613, 47649, 47685, 47720, 47756, 47792, 47827, 47863, 47898, 47934, 47969, 48004,
	48040, 48075, 48110, 48146, 48181, 48216, 48251, 48286, 48322, 48357, 48392, 48427, 48462, 48496, 48531,
	48566, 48601, 48636, 48671, 48705, 48740, 48775, 48809, 48844, 48878, 48913, 48947, 48982, 49016, 49051,
	49085, 49119, 49154, 49188, 49222, 49256, 49290, 49324, 49359, 49393, 49427, 49461, 49495, 49528, 49562,
	49596, 49630, 49664, 49698, 49731, 49765, 49799, 49832, 49866, 49899, 49933, 49966, 50000, 50033, 50067,
	50100, 50133, 50167, 50200, 50233, 50266, 50299, 50333, 50366, 50399, 50432, 50465, 50498, 50531, 50564,
	50597, 50629, 50662, 50695, 50728, 50760, 50793, 50826, 50858, 50891, 50924, 50956, 50989, 51021, 51053,
	51086, 51118, 51151, 51183, 51215, 51247, 51280, 51312, 51344, 51376, 51408, 51440, 51472 };

const uint16_t FACTORY_C2M_LOOKUP[1024] = {
	32, 32, 32, 32, 32, 32, 33, 33, 33, 34, 35, 35, 36, 37, 38, 39, 39, 41, 42, 43, 44, 45, 47, 48, 49, 51,
	53, 54, 56, 58, 60, 62, 63, 66, 68, 70, 72, 74, 77, 79, 81, 84, 87, 89, 92, 95, 98, 100, 103, 106, 110,
	113, 116, 119, 123, 126, 129, 133, 137, 140, 144, 148, 152, 155, 159, 163, 167, 172, 176, 180, 184, 189,
	193, 198, 202, 207, 212, 217, 221, 226, 231, 236, 241, 246, 252, 257, 262, 268, 273, 279, 284, 290, 295,
	301, 307, 313, 319, 325, 331, 337, 343, 350, 356, 362, 369, 375, 382, 388, 395, 402, 409, 415, 422, 429,
	436, 443, 451, 458, 465, 473, 480, 487, 495, 503, 510, 518, 526, 534, 542, 549, 558, 566, 574, 582, 590,
	599, 607, 615, 624, 633, 641, 650, 659, 667, 676, 685, 694, 703, 712, 722, 731, 740, 750, 759, 768, 778,
	788, 797, 807, 817, 827, 837, 847, 857, 867, 877, 887, 897, 908, 918, 928, 939, 950, 960, 971, 982, 992,
	1003, 1014, 1025, 1036, 1047, 1059, 1070, 1081, 1092, 1104, 1115, 1127, 1138, 1150, 1162, 1174, 1185, 1197,
	1209, 1221, 1233, 1245, 1258, 1270, 1282, 1294, 1307, 1319, 1332, 1344, 1357, 1370, 1383, 1395, 1408, 1421,
	1434, 1447, 1460, 1474, 1487, 1500, 1514, 1527, 1540, 1554, 1568, 1581, 1595, 1609, 1622, 1636, 1650, 1664,
	1678, 1692, 1707, 1721, 1735, 1749, 1764, 1778, 1793, 1807, 1822, 1837, 1852, 1866, 1881, 1896, 1911, 1926,
	1941, 1956, 1972, 1987, 2002, 2018, 2033, 2048, 2064, 2080, 2095, 2111, 2127, 2143, 2159, 2174, 2190, 2207,
	2223, 2239, 2255, 2271, 2288, 2304, 2321, 2337, 2354, 2370, 2387, 2404, 2420, 2437, 2454, 2471, 2488, 2505,
	2522, 2540, 2557, 2574, 2592, 2609, 2626, 2644, 2662, 2679, 2697, 2715, 2732, 2750, 2768, 2786, 2804, 2822,
	2840, 2859, 2877, 2895, 2913, 2932, 2950, 2969, 2987, 3006, 3025, 3043, 3062, 3081, 3100, 3119, 3138, 3157,
	3176, 3195, 3214, 3234, 3253, 3272, 3292, 3311, 3331, 3351, 3370, 3390, 3410, 3430, 3449, 3469, 3489, 3509,
	3529, 3550, 3570, 3590, 3610, 3631, 3651, 3672, 3692, 3713, 3733, 3754, 3775, 3795, 3816, 3837, 3858, 3879,
	3900, 3921, 3942, 3964, 3985, 4006, 4027, 4049, 4070, 4092, 4113, 4135, 4157, 4178, 4200, 4222, 4244, 4266,
	4288, 4310, 4332, 4354, 4376, 4399, 4421, 4443, 4466, 4488, 4510, 4533, 4556, 4578, 4601, 4624, 4647, 4669,
	4692, 4715, 4738, 4761, 4784, 4808, 4831, 4854, 4877, 4901, 4924, 4948, 4971, 4995, 5018, 5042, 5066, 5089,
	5113, 5137, 5161, 5185, 5209, 5233, 5257, 5281, 5306, 5330, 5354, 5378, 5403, 5427, 5452, 5476, 5501, 5526,
	5550, 5575, 5600, 5625, 5650, 5675, 5700, 5725, 5750, 5775, 5800, 5825, 5851, 5876, 5901, 5927, 5952, 5978,
	6003, 6029, 6055, 6080, 6106, 6132, 6158, 6184, 6210, 6236, 6262, 6288, 6314, 6340, 6367, 6393, 6419, 6446,
	6472, 6499, 6525, 6552, 6578, 6605, 6632, 6658, 6685, 6712, 6739, 6766, 6793, 6820, 6847, 6874, 6901, 6929,
	6956, 6983, 7011, 7038, 7066, 7093, 7121, 7148, 7176, 7204, 7231, 7259, 7287, 7315, 7343, 7371, 7399, 7427,
	7455, 7483, 7511, 7539, 7568, 7596, 7624, 7653, 7681, 7710, 7738, 7767, 7796, 7824, 7853, 7882, 7911, 7940,
	7968, 7997, 8026, 8055, 8085, 8114, 8143, 8172, 8201, 8231, 8260, 8289, 8319, 8348, 8378, 8407, 8437, 8467,
	8496, 8526, 8556, 8586, 8616, 8646, 8675, 8705, 8736, 8766, 8796, 8826, 8856, 8886, 8917, 8947, 8977, 9008,
	9038, 9069, 9099, 9130, 9161, 9191, 9222, 9253, 9284, 9315, 9345, 9376, 9407, 9438, 9469, 9501, 9532, 9563,
	9594, 9625, 9657, 9688, 9719, 9751, 9782, 9814, 9845, 9877, 9909, 9940, 9972, 10004, 10036, 10068, 10099,
	10131, 10163, 10195, 10227, 10259, 10292, 10324, 10356, 10388, 10421, 10453, 10485, 10518, 10550, 10583,
	10615, 10648, 10680, 10713, 10746, 10778, 10811, 10844, 10877, 10910, 10943, 10976, 11009, 11042, 11075,
	11108, 11141, 11174, 11208, 11241, 11274, 11308, 11341, 11374, 11408, 11441, 11475, 11509, 11542, 11576,
	11610, 11643, 11677, 11711, 11745, 11779, 11813, 11847, 11881, 11915, 11949, 11983, 12017, 12051, 12085,
	12120, 12154, 12188, 12223, 12257, 12292, 12326, 12361, 12395, 12430, 12465, 12499, 12534, 12569, 12604,
	12638, 12673, 12708, 12743, 12778, 12813, 12848, 12883, 12918, 12954, 12989, 13024, 13059, 13095, 13130,
	13165, 13201, 13236, 13272, 13307, 13343, 13379, 13414, 13450, 13486, 13521, 13557, 13593, 13629, 13665,
	13701, 13736, 13772, 13809, 13845, 13881, 13917, 13953, 13989, 14025, 14062, 14098, 14134, 14171, 14207,
	14244, 14280, 14317, 14353, 14390, 14426, 14463, 14500, 14536, 14573, 14610, 14647, 14684, 14721, 14758,
	14794, 14831, 14869, 14906, 14943, 14980, 15017, 15054, 15091, 15129, 15166, 15203, 15241, 15278, 15316,
	15353, 15391, 15428, 15466, 15503, 15541, 15579, 15616, 15654, 15692, 15730, 15767, 15805, 15843, 15881,
	15919, 15957, 15995, 16033, 16071, 16109, 16147, 16186, 16224, 16262, 16300, 16339, 16377, 16416, 16454,
	16492, 16531, 16569, 16608, 16646, 16685, 16724, 16762, 16801, 16840, 16879, 16917, 16956, 16995, 17034,
	17073, 17112, 17151, 17190, 17229, 17268, 17307, 17346, 17385, 17425, 17464, 17503, 17542, 17582, 17621,
	17660, 17700, 17739, 17779, 17818, 17858, 17897, 17937, 17977, 18016, 18056, 18096, 18135, 18175, 18215,
	18255, 18295, 18335, 18375, 18415, 18455, 18495, 18535, 18575, 18615, 18655, 18695, 18735, 18776, 18816,
	18856, 18896, 18937, 18977, 19017, 19058, 19098, 19139, 19179, 19220, 19260, 19301, 19342, 19382, 19423,
	19464, 19505, 19545, 19586, 19627, 19668, 19709, 19750, 19791, 19832, 19873, 19914, 19955, 19996, 20037,
	20078, 20119, 20160, 20202, 20243, 20284, 20325, 20367, 20408, 20450, 20491, 20532, 20574, 20615, 20657,
	20699, 20740, 20782, 20823, 20865, 20907, 20949, 20990, 21032, 21074, 21116, 21158, 21199, 21241, 21283,
	21325, 21367, 21409, 21451, 21493, 21536, 21578, 21620, 21662, 21704, 21746, 21789, 21831, 21873, 21916,
	21958, 22000, 22043, 22085, 22128, 22170, 22213, 22255, 22298, 22341, 22383, 22426, 22468, 22511, 22554,
	22597, 22639, 22682, 22725, 22768, 22811, 22854, 22897, 22940, 22983, 23026, 23069, 23112, 23155, 23198,
	23241, 23284, 23328, 23371, 23414, 23457, 23501, 23544, 23587, 23631, 23674, 23717, 23761, 23804, 23848,
	23891, 23935, 23978, 24022, 24066, 24109, 24153, 24197, 24240, 24284, 24328, 24372, 24415, 24459, 24503,
	24547, 24591, 24635, 24679, 24723, 24767, 24811, 24855, 24899, 24943, 24987, 25031, 25075, 25120, 25164,
	25208, 25252, 25297, 25341, 25385, 25430, 25474, 25518, 25563, 25607, 25652, 25696, 25741, 25785, 25830,
	25874, 25919, 25964, 26008, 26053, 26098, 26142, 26187, 26232, 26277, 26322, 26366, 26411, 26456, 26501,
	26546, 26591, 26636, 26681, 26726, 26771, 26816, 26861, 26906, 26951, 26997, 27042, 27087, 27132 };

// -------------------------------------------------------------------------------------------

extern value_type cell[], var[];
extern unsigned char* image_ptr;

// -------------------------------------------------------------------------------------------

void init_trigtab(void) {
#ifdef PARSERTEST
	return;
#else
	int i;

	if (gdata == NULL) return; // should not happen

	// Note: costab and tantab will not be freed, because we don't know if the user will call the plugin again

	if (gdata->costab == NULL) {
		gdata->costab = (double*)malloc(sizeof(double) * COSTABSIZE);
		if (gdata->costab) {
			for (i = 0; i < COSTABSIZE; ++i) {
				gdata->costab[i] = cos(FFANGLE(i));
			}
		}
	}

	if (gdata->tantab == NULL) {
		gdata->tantab = (double*)malloc(sizeof(double) * TANTABSIZE);
		if (gdata->tantab) {
			for (i = 0; i < TANTABSIZE; ++i) {
				if (i >= TANTABSIZE / 2) {
					/* the last '-1' in the expression '512-i-1' is for FilterFactory compatibility, and to avoid the undefined pi/2 area */
					gdata->tantab[i] = -gdata->tantab[TANTABSIZE - i - 1];
				} else {
					gdata->tantab[i] = tan(FFANGLE(i));
				}
			}
		}
	}
#endif
}

// -------------------------------------------------------------------------------------------

/**
Channel z for the input pixel at coordinates x,y.
Coordinates are relative to the input image data (pb->inData)
*/
static value_type rawsrc(value_type x, value_type y, value_type z) {
#ifdef PARSERTEST
	return 0;
#else
	long tmp1, tmp2;

	if (HAS_BIG_DOC(gpb)) {
		if (x < BIGDOC_IN_RECT(gpb).left) {
			x = BIGDOC_IN_RECT(gpb).left;
		} else if (x >= BIGDOC_IN_RECT(gpb).right) {
			x = BIGDOC_IN_RECT(gpb).right - 1;
		}
		if (y < BIGDOC_IN_RECT(gpb).top) {
			y = BIGDOC_IN_RECT(gpb).top;
		} else if (y >= BIGDOC_IN_RECT(gpb).bottom) {
			y = BIGDOC_IN_RECT(gpb).bottom - 1;
		}
		tmp1 = (long)gpb->inRowBytes * (y - BIGDOC_IN_RECT(gpb).top);
		tmp2 = (long)nplanes * (x - BIGDOC_IN_RECT(gpb).left) + z;
	} else {
		if (x < IN_RECT(gpb).left) {
			x = IN_RECT(gpb).left;
		} else if (x >= IN_RECT(gpb).right) {
			x = IN_RECT(gpb).right - 1;
		}
		if (y < IN_RECT(gpb).top) {
			y = IN_RECT(gpb).top;
		} else if (y >= IN_RECT(gpb).bottom) {
			y = IN_RECT(gpb).bottom - 1;
		}
		tmp1 = (long)gpb->inRowBytes * (y - IN_RECT(gpb).top);
		tmp2 = (long)nplanes* (x - IN_RECT(gpb).left) + z;
	}

	if (z < 0 || z >= var['Z']) return 0;

	switch (bytesPerPixelChannelIn) {
	case 1:
		return ((unsigned char*)gpb->inData)[tmp1+tmp2] - valueoffset_channel[z];
	case 2:
		return *((uint16_t*)(((unsigned char*)gpb->inData) + tmp1 + tmp2 * 2)) - valueoffset_channel[z];
	case 4:
		return (value_type)((float)maxChannelValueIn * *((float*)(((unsigned char*)gpb->inData) + tmp1 + tmp2 * 4)) - valueoffset_channel[z]);
	default:
		return 0;
	}
#endif
}

// -------------------------------------------------------------------------------------------

/**
src(x, y, z) Channel z for the pixel at coordinates x, y.
Coordinates are relative to filtered area (selection).
*/
value_type ff_src(value_type x, value_type y, value_type z) {
#ifdef PARSERTEST
	return 0;
#else
	if (x < 0) {
		x = 0;
	} else if (x >= var['X']) {
		x = var['X'] - 1;
	}

	if (y < 0) {
		y = 0;
	} else if (y >= var['Y']) {
		y = var['Y'] - 1;
	}

	if (z < 0 || z >= var['Z']) return 0;

	switch (bytesPerPixelChannelIn) {
	case 1:
		return image_ptr[(long)gpb->inRowBytes * y + (long)nplanes * x + z] - valueoffset_channel[z];
	case 2:
		return *((uint16_t*)(image_ptr + (long)gpb->inRowBytes * y + ((long)nplanes * x + z) * 2)) - valueoffset_channel[z];
	case 4:
		return (value_type)((float)maxChannelValueIn * *((float*)(image_ptr + (long)gpb->inRowBytes * y + ((long)nplanes * x + z) * 4)) - valueoffset_channel[z]);
	default:
		return 0;
	}
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_r2x(value_type d, value_type m) {
	// https://misc.daniel-marschall.de/projects/filter_factory/function_r2x.html
#ifdef PARSERTEST
	return 0;
#else
	int eax = m;
	int ebx = d;
	ebx &= 1023;
	ebx = FACTORY_COS_LOOKUP[ebx];
	return (((int64_t)eax * (int64_t)ebx) + 8191) >> 14;
#endif
}

value_type foundry_r2x(value_type d, value_type m) {
#ifdef PARSERTEST
	return 0;
#else
	return (value_type)RINT(m * gdata->costab[abs(d) % COSTABSIZE]);
#endif
}

/**
r2x(d,m) x displacement of the pixel m units away, at an angle of d,
from an arbitrary center
*/
value_type ff_r2x(value_type d, value_type m) {
#ifdef use_filterfactory_implementation_r2x
	return factory_r2x(d, m);
#else
	return foundry_r2x(d, m);
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_r2y(value_type d, value_type m) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/function_r2y.html
	return factory_r2x(d - 256, m);
#endif
}

value_type foundry_r2y(value_type d, value_type m) {
#ifdef PARSERTEST
	return 0;
#else
	return (value_type)RINT(m * gdata->costab[abs(d - 256) % COSTABSIZE]);
#endif
}

/**
r2y(d,m) y displacement of the pixel m units away, at an angle of d,
from an arbitrary center
*/
value_type ff_r2y(value_type d, value_type m) {
#ifdef use_filterfactory_implementation_r2y
	return factory_r2y(d, m);
#else
	return foundry_r2y(d, m);
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_rad(value_type d, value_type m, value_type z) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/function_rad.html

	const int xmin = 0;
	const int ymin = 0;
	//const int zmin = 0;

	int eax, ebx, ecx;

	ebx = FACTORY_COS_LOOKUP[d & 1023];
	eax = (((int64_t)m * (int64_t)ebx) + 8191) >> 14;
	ecx = ((var['X'] - xmin) >> 1) + eax;
	if (ecx < 0) {
		ecx = 0;
	} else {
		eax = var['X'];
		if (ecx >= eax) {
			ecx = eax - 1;
		}
	}

	ebx = FACTORY_COS_LOOKUP[(d - 256) & 1023];
	eax = (((int64_t)m * (int64_t)ebx) + 8191) >> 14;
	ebx = ((var['Y'] - ymin) >> 1) + eax;
	if (ebx < 0) {
		ebx = 0;
	} else {
		eax = var['Y'];
		if (ebx >= eax) {
			ebx = eax - 1;
		}
	}

	// Now return pixel [x=ecx,y=ebx,z=esi] from the source image!
	//return ff_src(ecx, ebx, z);
	//ebx *= gpb->inRowBytes;
	//ecx *= var['Z'] - zmin; // Z-zmin == nplanes
	//return image_ptr[z + ebx + ecx];

	if (z < 0 || z >= var['Z']) return 0;

	switch (bytesPerPixelChannelIn) {
	case 1:
		return image_ptr[ebx*gpb->inRowBytes + ecx*nplanes + z] - valueoffset_channel[z];
	case 2:
		return *((uint16_t*)(image_ptr + ebx*gpb->inRowBytes + (ecx*nplanes+z)*2)) - valueoffset_channel[z];
	case 4:
		return (value_type)((float)maxChannelValueIn * *((float*)(image_ptr + ebx*gpb->inRowBytes + (ecx*nplanes+z)*4)) - valueoffset_channel[z]);
	default:
		return 0;
	}
#endif
}

value_type foundry_rad(value_type d, value_type m, value_type z) {
#ifdef PARSERTEST
	return 0;
#else
	return ff_src(foundry_r2x(d, m) + var['X'] / 2, foundry_r2y(d, m) + var['Y'] / 2, z);
#endif
}

/**
rad(d,m,z) Channel z in the source image, which is m units away,
at an angle of d, from the center of the image
*/
value_type ff_rad(value_type d, value_type m, value_type z) {
#ifdef use_filterfactory_implementation_rad
	return factory_rad(d, m, z);
#else
	return foundry_rad(d, m, z);
#endif
}

// -------------------------------------------------------------------------------------------

/**
ctl(i) Value of slider i, where i is an integer between 0 and 7, inclusive
*/
value_type ff_ctl(value_type i) {
#ifdef PARSERTEST
	return 0;
#else
	return i >= 0 && i <= 7 ? gdata->parm.val[i] : 0;
#endif
}

// -------------------------------------------------------------------------------------------

value_type val_factory(value_type i, value_type a, value_type b) {
#ifdef PARSERTEST
	return 0;
#else
	if (i < 0 || i > 7) return 0;
	return ((long)(gdata->parm.val[i]) * (b - a)) / 255 + a;
#endif
}

/**
The only difference between `val_factory()` and `val_foundry()` is the handling of invalid values of "i"
*/
value_type val_foundry(value_type i, value_type a, value_type b) {
#ifdef PARSERTEST
	return 0;
#else
	return ((long)ff_ctl(i) * (b - a)) / 255 + a;
#endif
}

/*
val(i,a,b) Value of slider i, mapped onto the range a to b
*/
value_type ff_val(value_type i, value_type a, value_type b) {
#ifdef use_filterfactory_implementation_val
	return val_factory(i, a, b);
#else
	return val_foundry(i, a, b);
#endif
}

// -------------------------------------------------------------------------------------------

/*
map(i,n) Item n from mapping table i, where i is an integer between
0 and 3, inclusive, and n is and integer between 0 and 255,
inclusive
*/
value_type ff_map(value_type i, value_type n) {
#ifdef PARSERTEST
	return 0;
#else
	value_type H, L;
	const int i2 = i << 1;

	// This is how Filter Factory for Windows implements it:
	if (i < 0) return 0;
	if (i > 3) return 0;
	H = gdata->parm.val[i2]; // ctl(2i)
	L = gdata->parm.val[i2 + 1]; // ctl(2i+1)
	if (n < 0) n = 0;
	if (n > 255) n = 255; // Note: MacFF probably does "return 255" if n>255 (see testcases/map1_factory_win.png)

	if (H == L) {
		// This is undocumented in Filter Factory! (Taken from Windows implementation)
		if (n < H) return 0;
		if (n >= L) return 255;
	} else if (L > H) {
		// This is undocumented in Filter Factory! (Taken from Windows implementation)
		if (n <= H) return 255;
		if (n >= L) return 0;
	} else {
		if (n <= L) return 0;
		if (n >= H) return 255;
	}
	return (n - L) * 255 / (H - L);

	// This is the original formula used by GIMP User Filter v0.8 (uf-pcode.h).
	// It was used in Filter Foundry till version 1.7.0.16, inclusive.
	/*
	value_type H = ff_ctl(i * 2);
	value_type L = ff_ctl(i * 2 + 1);
	return abs(((long)n * (L - H) / 255) + H);
	*/
#endif

}

// -------------------------------------------------------------------------------------------

/**
min(a,b) Lesser of a and b
*/
value_type ff_min(value_type a, value_type b) {
#ifdef PARSERTEST
	return 0;
#else
	return a < b ? a : b;
#endif
}

// -------------------------------------------------------------------------------------------

/**
max(a,b) Greater of a and b
*/
value_type ff_max(value_type a, value_type b) {
#ifdef PARSERTEST
	return 0;
#else
	return a > b ? a : b;
#endif
}

// -------------------------------------------------------------------------------------------

/**
abs(a) Absolute value of a
*/
value_type ff_abs(value_type a) {
#ifdef PARSERTEST
	return 0;
#else
	return abs(a);
#endif
}

// -------------------------------------------------------------------------------------------

/**
add(a,b,c) Sum of a and b, or c, whichever is lesser
*/
value_type ff_add(value_type a, value_type b, value_type c) {
#ifdef PARSERTEST
	return 0;
#else
	return ff_min(a + b, c);
#endif
}

// -------------------------------------------------------------------------------------------

/**
sub(a,b,c) Difference of a and b, or c, whichever is greater
*/
value_type ff_sub(value_type a, value_type b, value_type c) {
#ifdef PARSERTEST
	return 0;
#else
	return ff_max(ff_dif(a, b), c);
#endif
}

// -------------------------------------------------------------------------------------------

/**
dif(a,b) Absolute value of the difference of a and b
*/
value_type ff_dif(value_type a, value_type b) {
#ifdef PARSERTEST
	return 0;
#else
	return abs(a - b);
#endif
}

// -------------------------------------------------------------------------------------------

struct factoryRngState {
	uint16_t index1;
	uint16_t index2;
	uint32_t seedTable[56];
	uint32_t seed;
	uint32_t seedSave;
} gFactoryRngState;

void factory_fill_rnd_lookup(uint32_t seed, struct factoryRngState* state) {
#ifdef PARSERTEST
	return;
#else
	// Algorithm of Filter Factory
	// Filter Factory uses Donald E.Knuth's subtractive
	// random number generator algorithm ("ran3"), which has been published
	// in Page 283 of "The Art of Computer Programming, volume 2: Seminumerical Algorithms",
	// Addison-Wesley, Reading, MA, second edition, 1981.
	// https://www.cec.uchile.cl/cinetica/pcordero/MC_libros/NumericalRecipesinC.pdf (PDF Page 307)

	long mj, mk;
	int i, ii, k;

	// 161803398 = 1.61803398 * 10^8 ~= phi * 10^8
	mj = 161803398 - (seed & 0x7fff);
	state->seedTable[55] = mj;

	mk = 1;
	ii = 0;
	for (i = 1; i <= 54; ++i) {
		if ((ii += 21) >= 55) ii -= 55; // ii = (21*i)%55;
		state->seedTable[ii] = mk;
		mk = mj - mk;
		mj = state->seedTable[ii];
	}

	for (k = 1; k <= 4; ++k) {
		ii = 30;
		for (i = 1; i <= 55; ++i) {
			if ((ii += 1) >= 55) ii -= 55;
			state->seedTable[i] -= state->seedTable[1 + ii]; // 1 + (i+30)%55
		}
	}

	state->seedSave = seed;

	return;
#endif
}

uint32_t factory_rnd(uint32_t a, uint32_t b, struct factoryRngState* state) {
#ifdef PARSERTEST
	return 0;
#else
	uint32_t mj; // Note: This must be "uint32_t". With "long" (as described by Knuth), it won't match FilterFactory's algorithm
	int range;

	if (state->seed != state->seedSave) {
		// (Intentional) behavior of Filter Foundry
		factory_fill_rnd_lookup(state->seed, &gFactoryRngState);
		state->index1 = 0;
		state->index2 = 31;
	}

	// Algorithm of Filter Factory
	// Filter Factory uses Donald E.Knuth's subtractive
	// random number generator algorithm ("ran3"), which has been published
	// in Page 283 of "The Art of Computer Programming, volume 2: Seminumerical Algorithms",
	// Addison-Wesley, Reading, MA, second edition, 1981.
	// https://www.cec.uchile.cl/cinetica/pcordero/MC_libros/NumericalRecipesinC.pdf (PDF Page 307)

	if (++state->index1 == 56) state->index1 = 1;
	if (++state->index2 == 56) state->index2 = 1;

	mj = state->seedTable[state->index1] -
	     state->seedTable[state->index2];
	state->seedTable[state->index1] = mj;

	// This is Filter Factory specific:
	// Reduce result into interval [a..b] by applying (a + (mj % (b - a + 1))
	// Try to avoid modulo in order to increase performance
	range = b - a;
	if (range < 0) return 0;
	switch (range) {
	case 255:
		return a + (mj & 0xFF);
	case 127:
		return a + (mj & 0x7F);
	case 63:
		return a + (mj & 0x3F);
	case 31:
		return a + (mj & 0x1F);
	case 15:
		return a + (mj & 0xF);
	case 7:
		return a + (mj & 0x7);
	case 3:
		return a + (mj & 0x3);
	case 1:
		return a + (mj & 0x1);
	case 0:
		return a;
	default:
		return a + (mj % (range + 1));
	}
#endif
}

value_type foundry_rnd(value_type a, value_type b) {
#ifdef PARSERTEST
	return 0;
#else
	return (int)((abs(a - b) + 1) * (rand() / (RAND_MAX + 1.))) + ff_min(a, b);
	//	return ((unsigned)rand() % (ff_dif(a,b)+1)) + ff_min(a,b);
#endif
}

/**
rnd(a,b) Random number between a and b, inclusive
*/
value_type ff_rnd(value_type a, value_type b) {
#ifdef use_filterfactory_implementation_rnd
	return factory_rnd(a, b, &gFactoryRngState);
#else
	return foundry_rnd(a, b);
#endif
}

// -------------------------------------------------------------------------------------------

int32_t factory_rst(uint32_t seed, struct factoryRngState* state) {
#ifdef PARSERTEST
	return 0;
#else
	// Attention: This is NOT the FilterFactory rst() implementation!

	// We implement rst(i) completely differently in Filter Foundry:
	// Every call of rst() will renew the lookup table.
	// In Filter Factory, there are strange/buggy things going
	// on: rst(i) only sets a seed and the lookup table is renewed
	// at the NEXT invocation of the filter. Furthermore, in FilterFactory,
	// the state is not reset between invocations, therefore, the preview image
	// will influence the PRNG state of the final image...
	// More information at "Filter Factory Compatibility.md"

	state->seed = seed;

	// Force renewal of the PRNG state in the next rnd(a,b) call.
	// This allows us to use:
	//    (x==0?rst(1):0), rnd(0,255)
	// But it is slower and this won't work anymore:
	//    rst(0), rnd(0,255)
	state->seedSave = seed + 1;

	return 0;
#endif
}

value_type foundry_rst(value_type seed) {
#ifdef PARSERTEST
	return 0;
#else
	srand(seed);
	return 0;
#endif
}

/**
rst(i) sets a random seed and returns 0. (undocumented Filter Factory function).
Added by Daniel Marschall, 18 Dec 2018
*/
value_type ff_rst(value_type seed) {
#ifdef use_filterfactory_implementation_rnd
	return factory_rst(seed, &gFactoryRngState);
#else
	return foundry_rst(seed);
#endif
}

// -------------------------------------------------------------------------------------------

void factory_initialize_rnd_variables(void) {
#ifdef PARSERTEST
	return;
#else
	gFactoryRngState.seed = 0; // default seed
	gFactoryRngState.seedSave = gFactoryRngState.seed + 1; // force rnd() to call factory_fill_rnd_lookup()
#endif
}

void foundry_initialize_rnd_variables(void) {
#ifdef PARSERTEST
	return;
#else
	foundry_rst(691204);
#endif
}

void initialize_rnd_variables(void) {
#ifdef use_filterfactory_implementation_rnd
	factory_initialize_rnd_variables();
#else
	foundry_initialize_rnd_variables();
#endif
}

// -------------------------------------------------------------------------------------------

/**
mix(a,b,n,d) Mixture of a and b by fraction n/d, a*n/d+b*(d-n)/d
*/
value_type ff_mix(value_type a, value_type b, value_type n, value_type d) {
#ifdef PARSERTEST
	return 0;
#else
	if (d == 0) return 0;
	return ((long)a * n) / d + ((long)b * (d - n)) / d;
#endif
}

// -------------------------------------------------------------------------------------------

/**
scl(a,il,ih,ol,oh) Scale a from input range (il to ih)
to output range (ol to oh) */
value_type ff_scl(value_type a, value_type il, value_type ih,
	value_type ol, value_type oh) {
#ifdef PARSERTEST
	return 0;
#else
	return ih == il ? 0 : ol + ((long)(oh - ol) * (a - il)) / (ih - il);
#endif
}

// -------------------------------------------------------------------------------------------

/**
pow(b,e) Calculates the base to the exponent power, that is, b^e.
Added by Daniel Marschall.
*/
value_type ff_pow(value_type b, value_type e) {
#ifdef PARSERTEST
	return 0;
#else
	double a = pow((double)b, (double)e);
	if (a - floor(a) >= 0.5)
		return (value_type)(floor(a) + 1);
	else
		return (value_type)(floor(a));
#endif
}

// -------------------------------------------------------------------------------------------

static uint32_t isqrt(uint32_t x) {
#ifdef PARSERTEST
	return 0;
#else
	// based on https://gist.github.com/orlp/3481770

	static uint32_t lkpSquares[65535];
	static int lkpInitialized = 0;
	const uint32_t* p;
	int i;

	while (lkpInitialized == 1) { /* If other thread is currently creating the lookup table, then wait */ }
	if (!lkpInitialized) {
		lkpInitialized = 1;
		for (i = 0; i < 65535; ++i) {
			lkpSquares[i] = i * i;
		}
		lkpInitialized = 2;
	}

	p = lkpSquares;

	if (p[32768] <= x) p += 32768;
	if (p[16384] <= x) p += 16384;
	if (p[8192] <= x) p += 8192;
	if (p[4096] <= x) p += 4096;
	if (p[2048] <= x) p += 2048;
	if (p[1024] <= x) p += 1024;
	if (p[512] <= x) p += 512;
	if (p[256] <= x) p += 256;
	if (p[128] <= x) p += 128;
	if (p[64] <= x) p += 64;
	if (p[32] <= x) p += 32;
	if (p[16] <= x) p += 16;
	if (p[8] <= x) p += 8;
	if (p[4] <= x) p += 4;
	if (p[2] <= x) p += 2;
	if (p[1] <= x) p += 1;

	return (uint32_t)(p - lkpSquares);
#endif
}

value_type factory_sqr(value_type x) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/function_sqr.html

	int eax, ebx, ecx;

	ebx = x;
	if (ebx > 1) {
		ecx = ebx;
		ebx = ebx >> 1;
		eax = 2;
		while (ebx > eax) {
			eax = ecx;
			eax /= ebx;
			ebx += eax;
			ebx = ebx >> 1;
		}
	}
	return ebx;
#endif
}

value_type foundry_sqr(value_type x) {
#ifdef PARSERTEST
	return 0;
#else
	// Note: FilterFactory has sqr(x)=x if x<0 . Here we set sqr(x)=0 for x<0
	return x < 0 ? 0 : isqrt(x);
#endif
}

/**
sqr(x) Square root of x
*/
value_type ff_sqr(value_type x) {
#ifdef use_filterfactory_implementation_sqr
	return factory_sqr(x);
#else
	return foundry_sqr(x);
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_cos(value_type x) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/function_cos.html
	int res;
	if (x < 0) x = -x;
	x &= 0x3ff; // 1023
	res = FACTORY_COS_LOOKUP[x];
	return res >= 0 ? (res / 32) : res / 32 - 1;
#endif
}

value_type foundry_cos(value_type x) {
#ifdef PARSERTEST
	return 0;
#else
	//return RINT(TRIGAMP*cos(FFANGLE(x)));
	return (value_type)RINT(TRIGAMP * gdata->costab[abs(x) % COSTABSIZE]);
#endif
}

/**
cos(x) Cosine function of x, where x is an integer between 0 and
1024, inclusive, and the value returned is an integer
between -512 and 512, inclusive (Windows) or -1024 and
1024, inclusive (Mac OS)
*/
value_type ff_cos(value_type x) {
#ifdef use_filterfactory_implementation_cos
	return factory_cos(x);
#else
	return foundry_cos(x);
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_sin(value_type x) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/function_sin.html
	return factory_cos(x - 256);
#endif
}

value_type foundry_sin(value_type x) {
#ifdef PARSERTEST
	return 0;
#else
	//return RINT(TRIGAMP*sin(FFANGLE(x)));
	return foundry_cos(x - 256);
#endif
}

/**
sin(x) Sine function of x, where x is an integer between 0 and
1024, inclusive, and the value returned is an integer
between -512 and 512, inclusive (Windows) or -1024 and
1024, inclusive (Mac OS)
*/
value_type ff_sin(value_type x) {
#ifdef use_filterfactory_implementation_sin
	return factory_sin(x);
#else
	return foundry_sin(x);
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_tan(value_type x) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/function_tan.html
	int v1 = x;
	int v2 = v1 < 0 ? -v1 : v1;
	v2 &= 0x3ff; // 1023
	v2 = FACTORY_COS_LOOKUP[v2];
	v1 -= 256;
	if (v1 < 0) v1 = -v1;
	v1 &= 0x3ff; // 1023
	v1 = FACTORY_COS_LOOKUP[v1];
	if (v2 == 0) return 0;
	v1 = v1 << 10; // v1 *= 1024;
	return v1 / v2;
#endif
}

value_type foundry_tan(value_type x) {
#ifdef PARSERTEST
	return 0;
#else
	// The following filter shows that the Filter Factory manual differs from the implementation.
	//     R = cos(x) > 1024 || cos(x) < -1024 || cos(-x) > 1024 || cos(-x) < -1024 ? 255 : 0
	//     G = tan(x) > 1024 || tan(x) < -1024 || tan(-x) > 1024 || tan(-x) < -1024 ? 255 : 0
	//     B = sin(x) > 1024 || sin(x) < -1024 || sin(-x) > 1024 || sin(-x) < -1024 ? 255 : 0
	// It outputs green stripes, showing that the output of tan() is not bounded.
	// So, we do it the same way to stay compatible.
	if (x < 0) x--; /* required for Filter Factory compatibility */
	while (x < 0) x += TANTABSIZE;
	return (value_type)RINT(2 * TRIGAMP * gdata->tantab[x % TANTABSIZE]); // We need the x2 multiplicator for some reason
#endif
}

/**
tan(x) Tangent function of x, where x is an integer
between -256 and 256, inclusive. Althought the Filter Factory manual
stated that the return value is bounded to -512 and 512, inclusive (Windows) or
-1024 and 1024, inclusive (Mac OS), the output is actually NOT bounded!
*/
value_type ff_tan(value_type x) {
#ifdef use_filterfactory_implementation_tan
	return factory_tan(x);
#else
	return foundry_tan(x);
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_c2d(value_type x, value_type y) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/function_c2d.html
	int32_t eax, ebx, ecx;
	eax = y;
	ebx = x;
	ecx = 0;
	if (eax < 0) {
		eax = -eax;
		ecx |= 4/*0b0100*/;
	}
	if (ebx < 0) {
		ebx = -ebx;
		ecx |= 3/*0b0011*/;
	}
	if (eax > ebx) {
		int tmp;
		ecx ^= 1/*0b0001*/;
		tmp = eax;
		eax = ebx;
		ebx = tmp;
	}
	if (eax > 0) {
		eax = eax << 10;
		eax /= ebx;
		if (eax != 0) {
			eax = (eax & 0xFFFF0000ul) | (FACTORY_C2D_LOOKUP[eax - 1] & 0xFFFF);
			eax = eax << 9;
			ebx = 205888; // 205888/65536 == pi == 3.14159265358979323846264338327
			eax /= ebx;
		}
	}
	if ((ecx & 1/*0b0001*/) != 0) {
		eax = -eax;
		eax += 256;
	}
	if ((ecx & 2/*0b0010*/) != 0) {
		eax += 256;
	}
	if ((ecx & 4/*0b0100*/) != 0) {
		eax = -eax;
	}
	return eax;
#endif
}

value_type foundry_c2d(value_type x, value_type y) {
#ifdef PARSERTEST
	return 0;
#else
	// Behavior of FilterFoundry <1.7:
	//return foundry_c2d_negated(x,y);

	// Behavior in FilterFoundry 1.7+: Matches FilterFactory
	return (value_type)RINT(TO_FFANGLE(atan2(y, x)));
#endif
}

/**
c2d(x,y) Angle displacement of the pixel at coordinates x,y
*/
value_type ff_c2d(value_type x, value_type y) {
#ifdef use_filterfactory_implementation_c2d
	return factory_c2d(x, y);
#else
	return foundry_c2d(x, y);
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_c2m(value_type x, value_type y) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/function_c2m.html
	int32_t eax, ebx;
	ebx = y < 0 ? -y : y;
	eax = x < 0 ? -x : x;
	if (eax == ebx) {
		eax = 27146; // 27146/65536 == sqrt(2)-1 == 0.41421356237
	} else {
		if (eax > ebx) {
			int tmp = eax;
			eax = ebx;
			ebx = tmp;
		}
		eax = eax << 10;
		eax /= ebx;
		eax = FACTORY_C2M_LOOKUP[eax];
	}
	eax = ((int64_t)eax * (int64_t)ebx) >> 16;
	eax += ebx;
	return eax;
#endif
}

value_type foundry_c2m(value_type x, value_type y) {
#ifdef PARSERTEST
	return 0;
#else
	return isqrt((long)x * x + (long)y * y);
#endif
}

/**
c2m(x,y) Magnitude displacement of the pixel at coordinates x,y
*/
value_type ff_c2m(value_type x, value_type y) {
#ifdef use_filterfactory_implementation_c2m
	return factory_c2m(x, y);
#else
	return foundry_c2m(x, y);
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_d(void) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/symbol_d_lowercase.html

	int eax, ebx, ecx;
	const int xmin = 0, ymin = 0;

	eax = -(var['Y'] - ymin) / 2;
	ebx = -(var['X'] - xmin) / 2;
	ecx = 0;
	eax += var['y'];
	if (eax < 0) {
		eax = -eax;
		ecx |= 4/*0b0100*/;
	}
	ebx += var['x'];
	if (ebx < 0) {
		ebx = -ebx;
		ecx |= 3/*0b0011*/;
	}
	if (eax > ebx) {
		int tmp;
		ecx ^= 1/*0b0001*/;
		tmp = eax;
		eax = ebx;
		ebx = tmp;
	}
	if (eax > 0) {
		eax = eax << 10;
		eax /= ebx;
		if (eax != 0) { // C2D_LOOKUP[-1] will never be called. Good!
			eax = (eax & 0xFFFF0000ul) + (FACTORY_C2D_LOOKUP[eax - 1] & 0xFFFF);
			eax = eax << 9;
			ebx = 205888; // 205888/65536 == pi == 3.14159265358979323846264338327
			eax /= ebx;
		}
	}
	if ((ecx & 1/*0b0001*/) != 0) {
		eax = -eax;
		eax += 256;
	}
	if ((ecx & 2/*0b0010*/) != 0) {
		eax += 256;
	}
	if ((ecx & 4/*0b0100*/) != 0) {
		eax = -eax;
	}
	return eax;
#endif
}

value_type foundry_c2d_negated(int x, int y) {
#ifdef PARSERTEST
	return 0;
#else
	return (value_type)RINT(TO_FFANGLE(atan2(-y, -x)));
#endif
}

value_type foundry_d(void) {
#ifdef PARSERTEST
	return 0;
#else
	// NOTE: FilterFactory uses c2d(x,y):=atan2(y,x), but d:=atan2(-y,-x)
	// Due to compatibility reasons, we implement it the same way!
	// Sign of y difference is negated, as we are dealing with top-down coordinates angle is "observed"
	int x = var['X'] / 2 - var['x'];
	int y = var['Y'] / 2 - var['y'];
	return foundry_c2d_negated(x, y);
#endif
}

/**
Direction(angle) of the current pixel from the center of the image,
where d is an integer between -512 and 512 inclusive
*/
value_type ff_d(void) {
#ifdef use_filterfactory_implementation_d
	// Output range: -512 ... 512
	return factory_d();
#else
	// Output range: -511 ... 512
	return foundry_d();
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_M(void) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/symbol_m_uppercase.html

	int eax, ebx;
	const int xmin = 0, ymin = 0;

	eax = (var['X'] - xmin) >> 1;
	ebx = (var['Y'] - ymin) >> 1;
	if (eax == ebx) {
		eax = 27146; // 27146/65536 == sqrt(2)-1 == 0.41421356237
	} else {
		if (eax > ebx) {
			int tmp = eax;
			eax = ebx;
			ebx = tmp;
		}
		eax = eax << 10;
		eax /= ebx;
		eax = FACTORY_C2M_LOOKUP[eax];
	}
	eax = ((int64_t)eax * (int64_t)ebx) >> 16;
	eax += ebx;
	return eax;
#endif
}

value_type foundry_M(void) {
#ifdef PARSERTEST
	return 0;
#else
	return foundry_c2m(var['X'], var['Y']) / 2;
#endif
}

/**
Range of magnitudes with the image, where M is one half the diagonal size of the image
*/
value_type ff_M(void) {
#ifdef use_filterfactory_implementation_M
	return factory_M();
#else
	return foundry_M();
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_m(void) {
#ifdef PARSERTEST
	return 0;
#else
	// https://misc.daniel-marschall.de/projects/filter_factory/symbol_m_lowercase.html

	int eax, ebx;
	const int xmin = 0, ymin = 0;

	eax = ((xmin - var['X']) >> 1) + var['x'];
	ebx = ((ymin - var['Y']) >> 1) + var['y'];
	eax = eax < 0 ? -eax : eax;
	ebx = ebx < 0 ? -ebx : ebx;

	if (eax == ebx) {
		eax = 27146; // 27146/65536 == sqrt(2)-1 == 0.41421356237
	} else {
		if (eax > ebx) {
			int tmp = eax;
			eax = ebx;
			ebx = tmp;
		}
		eax = FACTORY_C2M_LOOKUP[1024 * eax / ebx];
	}
	eax = ((int64_t)eax * (int64_t)ebx) >> 16;
	eax += ebx;
	return eax;
#endif
}

value_type foundry_m(void) {
#ifdef PARSERTEST
	return 0;
#else
	return foundry_c2m(var['X'] / 2 - var['x'], var['Y'] / 2 - var['y']);
#endif
}

/**
Distance (magnitude) from the center of the image to the current pixel
*/
value_type ff_m(void) {
#ifdef use_filterfactory_implementation_m
	return factory_m();
#else
	return foundry_m();
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_i(void) {
#ifdef PARSERTEST
	return 0;
#else
	return ((76L * var['r']) + (150L * var['g']) + (29L * var['b'])) / 256; // range: [0..254]
#endif
}

value_type foundry_i(void) {
#ifdef PARSERTEST
	return 0;
#else
	// These formulas are more accurate, e.g. pure white has now i=255 instead of 254
	return ((299L * var['r']) + (587L * var['g']) + (114L * var['b'])) / 1000;    // range: [0..255]
#endif
}

/**
"Y" value of the YUV color-space
*/
value_type ff_i(void) {
#ifdef use_filterfactory_implementation_i
	return factory_i();
#else
	return foundry_i();
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_u(void) {
#ifdef PARSERTEST
	return 0;
#else
	return ((-19L * var['r']) + (-37L * var['g']) + (56L * var['b'])) / 256; // range: [-55..55]
#endif
}

value_type foundry_u(void) {
#ifdef PARSERTEST
	return 0;
#else
	// These formulas are more accurate, e.g. pure white has now i=255 instead of 254
	return ((-147407L * var['r']) + (-289391L * var['g']) + (436798L * var['b'])) / 2000000; // range: [-55..55]
#endif
}

/**
"U" value of the YUV color-space
*/
value_type ff_u(void) {
#ifdef use_filterfactory_implementation_u
	return factory_u();
#else
	return foundry_u();
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_v(void) {
#ifdef PARSERTEST
	return 0;
#else
	return ((78L * var['r']) + (-65L * var['g']) + (-13L * var['b'])) / 256; // range: [-77..77]
#endif
}

value_type foundry_v(void) {
#ifdef PARSERTEST
	return 0;
#else
	// These formulas are more accurate, e.g. pure white has now i=255 instead of 254
	return ((614777L * var['r']) + (-514799L * var['g']) + (-99978L * var['b'])) / 2000000; // range: [-78..78]
#endif
}

/**
"V" value of the YUV color-space
*/
value_type ff_v(void) {
#ifdef use_filterfactory_implementation_v
	return factory_v();
#else
	return foundry_v();
#endif
}

// -------------------------------------------------------------------------------------------

value_type factory_get(value_type i) {
#ifdef PARSERTEST
	return 0;
#else
	return i >= 0 && i < NUM_CELLS ? cell[i] : i;
#endif
}

value_type foundry_get(value_type i) {
#ifdef PARSERTEST
	return 0;
#else
	return i >= 0 && i < NUM_CELLS ? cell[i] : 0;
#endif
}

/**
get(i) Returns the current cell value at i
*/
value_type ff_get(value_type i) {
	// The only difference is the handling of invalid values of "i"
#ifdef use_filterfactory_implementation_get
	return factory_get(i);
#else
	return foundry_get(i);
#endif
}

// -------------------------------------------------------------------------------------------

/**
put(v,i) Puts the new value v into cell i
*/
value_type ff_put(value_type v, value_type i) {
#ifdef PARSERTEST
	return 0;
#else
	if (i >= 0 && i < NUM_CELLS)
		cell[i] = v;
	return v;
#endif
}

// -------------------------------------------------------------------------------------------

/**
Convolve. Applies a convolution matrix and divides with d.
*/
value_type ff_cnv(value_type m11, value_type m12, value_type m13,
	value_type m21, value_type m22, value_type m23,
	value_type m31, value_type m32, value_type m33,
	value_type d)
{
#ifdef PARSERTEST
	return 0;
#else
	long total;
	int x, y, z;

	if (d == 0) return 0;

	// shift x,y from selection-relative to image relative required by rawsrc()
	if (HAS_BIG_DOC(gpb)) {
		x = var['x'] + BIGDOC_FILTER_RECT(gpb).left;
		y = var['y'] + BIGDOC_FILTER_RECT(gpb).top;
	} else {
		x = var['x'] + FILTER_RECT(gpb).left;
		y = var['y'] + FILTER_RECT(gpb).top;
	}
	z = var['z'];

	// rawsrc() will choose the neighbor pixels if x/y goes out-of-bounds (outer border pixels)
	if (z >= 0 && z < var['Z'])
		total = m11 * rawsrc(x - 1, y - 1, z) + m12 * rawsrc(x, y - 1, z) + m13 * rawsrc(x + 1, y - 1, z)
		+ m21 * rawsrc(x - 1, y, z) + m22 * rawsrc(x, y, z) + m23 * rawsrc(x + 1, y, z)
		+ m31 * rawsrc(x - 1, y + 1, z) + m32 * rawsrc(x, y + 1, z) + m33 * rawsrc(x + 1, y + 1, z);
	else
		total = 0; // ... can this happen at all ?!

	return total / d;
#endif
}

// -------------------------------------------------------------------------------------------

value_type zero_val = 0;
value_type one_val = 1;

// In Lab color space, a and b are -128..127 for 8-bit and -16384..16256 for 16-bit.
// These variables are set in setup() and help making evalpixel() faster
value_type valueoffset_channel[4]; // will be set by process.c (setup)
value_type min_channel_val[4]; // will be set by process.c (setup)
value_type min_val_c; // will be set by process.c (evalpixel)
value_type max_channel_val[4]; // will be set by process.c (setup)
value_type max_val_c; // will be set by process.c (evalpixel)

value_type min_val_i; // will be set by process.c (setup)
value_type max_val_i; // will be set by process.c (setup)

value_type min_val_u_factory = 0;
value_type max_val_u_factory = 255;
value_type min_val_u; // will be set by process.c (setup)
value_type max_val_u; // will be set by process.c (setup)

value_type min_val_v_factory = 0;
value_type max_val_v_factory = 255;
value_type min_val_v; // will be set by process.c (setup)
value_type max_val_v; // will be set by process.c (setup)

value_type min_val_d_factory = 0;
value_type max_val_d_factory = 1024;
value_type min_val_d = -512;
value_type max_val_d = 512;

/**
predefined symbols
*/
struct sym_rec predefs[] = {
	/* functions */

	{0,TOK_FN3,"src", (pfunc_type)ff_src, 0},
	{0,TOK_FN3,"rad", (pfunc_type)ff_rad, 0},
	{0,TOK_FN1,"ctl", (pfunc_type)ff_ctl, 0},
	{0,TOK_FN3,"val", (pfunc_type)ff_val, 0},
	{0,TOK_FN2,"map", (pfunc_type)ff_map, 0},
	{0,TOK_FN2,"min", (pfunc_type)ff_min, 0},
	{0,TOK_FN2,"max", (pfunc_type)ff_max, 0},
	{0,TOK_FN1,"abs", (pfunc_type)ff_abs, 0},
	{0,TOK_FN3,"add", (pfunc_type)ff_add, 0},
	{0,TOK_FN3,"sub", (pfunc_type)ff_sub, 0},
	{0,TOK_FN2,"dif", (pfunc_type)ff_dif, 0},
	{0,TOK_FN2,"rnd", (pfunc_type)ff_rnd, 0},
	{0,TOK_FN4,"mix", (pfunc_type)ff_mix, 0},
	{0,TOK_FN5,"scl", (pfunc_type)ff_scl, 0},
	{0,TOK_FN1,"sqr", (pfunc_type)ff_sqr, 0},
	{0,TOK_FN1,"sqrt", (pfunc_type)ff_sqr, 0}, // sqrt() is synonym to sqr() in Premiere
	{0,TOK_FN1,"sin", (pfunc_type)ff_sin, 0},
	{0,TOK_FN1,"cos", (pfunc_type)ff_cos, 0},
	{0,TOK_FN1,"tan", (pfunc_type)ff_tan, 0},
	{0,TOK_FN2,"r2x", (pfunc_type)ff_r2x, 0},
	{0,TOK_FN2,"r2y", (pfunc_type)ff_r2y, 0},
	{0,TOK_FN2,"c2d", (pfunc_type)ff_c2d, 0},
	{0,TOK_FN2,"c2m", (pfunc_type)ff_c2m, 0},
	{0,TOK_FN1,"get", (pfunc_type)ff_get, 0},
	{0,TOK_FN2,"put", (pfunc_type)ff_put, 0},
	{0,TOK_FN10,"cnv",(pfunc_type)ff_cnv, 0},
	{0,TOK_FN1,"rst", (pfunc_type)ff_rst, 0}, // undocumented FilterFactory function
	{0,TOK_FN2,"pow", (pfunc_type)ff_pow, 0}, // new function, also added in inofficial Filter Factory 3.1.0 patch

	/* Predefined variables (names with more than 1 character); most of them are undocumented in Filter Factory */
	/* The predefined variables with 1 character are defined in lexer.l and process.c */
	/* In this table, you must not add TOK_VAR with only 1 character (since this case is not defined in parser.y) */

	{0,TOK_VAR,"rmax",0, &max_channel_val[0]}, // alias of R (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"gmax",0, &max_channel_val[1]}, // alias of G (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"bmax",0, &max_channel_val[2]}, // alias of B (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"amax",0, &max_channel_val[3]}, // alias of A (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"cmax",0, &max_val_c}, // alias of C (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"imax",0, &max_val_i},

#ifdef use_filterfactory_implementation_u_minmax
	{0,TOK_VAR,"umax",0, &max_val_u_factory},
#else
	{0,TOK_VAR,"umax",0, &max_val_u},
#endif

#ifdef use_filterfactory_implementation_v_minmax
	{0,TOK_VAR,"vmax",0, &max_val_v_factory},
#else
	{0,TOK_VAR,"vmax",0, &max_val_v},
#endif

#ifdef use_filterfactory_implementation_d_minmax
	{0,TOK_VAR,"dmax",0, &max_val_d_factory},
#else
	{0,TOK_VAR,"dmax",0, &max_val_d},
#endif

	{0,TOK_VAR,"mmax",0, &var['M']}, // alias of M (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"pmax",0, &var['Z']}, // alias of P (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"xmax",0, &var['X']}, // alias of X (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"ymax",0, &var['Y']}, // alias of Y (defined in lexer.l and set by process.c)
	{0,TOK_VAR,"zmax",0, &var['Z']}, // alias of Z (defined in lexer.l and set by process.c)

	{0,TOK_VAR,"rmin",0, &min_channel_val[0]},
	{0,TOK_VAR,"gmin",0, &min_channel_val[1]},
	{0,TOK_VAR,"bmin",0, &min_channel_val[2]},
	{0,TOK_VAR,"amin",0, &min_channel_val[3]},
	{0,TOK_VAR,"cmin",0, &min_val_c},
	{0,TOK_VAR,"imin",0, &min_val_i},

#ifdef use_filterfactory_implementation_u_minmax
	{0,TOK_VAR,"umin",0, &min_val_u_factory},
#else
	{0,TOK_VAR,"umin",0, &min_val_u},
#endif

#ifdef use_filterfactory_implementation_v_minmax
	{0,TOK_VAR,"vmin",0, &min_val_v_factory},
#else
	{0,TOK_VAR,"vmin",0, &min_val_v},
#endif

#ifdef use_filterfactory_implementation_d_minmax
	{0,TOK_VAR,"dmin",0, &min_val_d_factory},
#else
	{0,TOK_VAR,"dmin",0, &min_val_d},
#endif

	{0,TOK_VAR,"mmin",0, &zero_val},
	{0,TOK_VAR,"pmin",0, &zero_val},
	{0,TOK_VAR,"xmin",0, &zero_val},
	{0,TOK_VAR,"ymin",0, &zero_val},
	{0,TOK_VAR,"zmin",0, &zero_val},

	/* Undocumented synonyms of FilterFactory for compatibility with Premiere */
	{0,TOK_FN10,"cnv0",(pfunc_type)ff_cnv, 0},
	{0,TOK_FN3,"src0", (pfunc_type)ff_src, 0},
	{0,TOK_FN3,"rad0", (pfunc_type)ff_rad, 0},
	{0,TOK_FN10,"cnv1",(pfunc_type)ff_cnv, 0},
	{0,TOK_FN3,"src1", (pfunc_type)ff_src, 0},
	{0,TOK_FN3,"rad1", (pfunc_type)ff_rad, 0},
	{0,TOK_VAR,"r0",0, &var['r']},
	{0,TOK_VAR,"g0",0, &var['g']},
	{0,TOK_VAR,"b0",0, &var['b']},
	{0,TOK_VAR,"a0",0, &var['a']},
	{0,TOK_VAR,"c0",0, &var['c']},
	{0,TOK_VAR,"i0",0, &var['i']},
	{0,TOK_VAR,"u0",0, &var['u']},
	{0,TOK_VAR,"v0",0, &var['v']},
	{0,TOK_VAR,"d0",0, &var['d']},
	{0,TOK_VAR,"m0",0, &var['m']},
	{0,TOK_VAR,"r1",0, &var['r']},
	{0,TOK_VAR,"g1",0, &var['g']},
	{0,TOK_VAR,"b1",0, &var['b']},
	{0,TOK_VAR,"a1",0, &var['a']},
	{0,TOK_VAR,"c1",0, &var['c']},
	{0,TOK_VAR,"i1",0, &var['i']},
	{0,TOK_VAR,"u1",0, &var['u']},
	{0,TOK_VAR,"v1",0, &var['v']},
	{0,TOK_VAR,"d1",0, &var['d']},
	{0,TOK_VAR,"m1",0, &var['m']},
	{0,TOK_VAR,"tmin",0, &zero_val},
	{0,TOK_VAR,"tmax",0, &one_val},
	{0,TOK_VAR,"total",0, &one_val},

	{0,0,0,0,0}
};
