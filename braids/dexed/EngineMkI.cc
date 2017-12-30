/*
 * Copyright (C) 2015-2017 Pascal Gauthier.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 * The code is based on ppplay https://github.com/stohrendorf/ppplay and opl3
 * math documentation :
 * https://github.com/gtaylormb/opl3_fpga/blob/master/docs/opl3math/opl3math.pdf
 *
 */

#include "EngineMkI.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>

#include "sin.h"
#include "exp2.h"

#ifdef DEBUG
    #include "time.h"
    //#define MKIDEBUG
#endif

#ifdef _WIN32
#if _MSC_VER < 1800
    double log2(double n)  {  
        return log(n) / log(2.0);  
    }
    double round(double n) {
        return n < 0.0 ? ceil(n - 0.5) : floor(n + 0.5);
    }
#endif
    __declspec(align(16)) const int zeros[N] = {0};
#else
    const int32_t __attribute__ ((aligned(16))) zeros[N] = {0};
#endif

static const uint16_t NEGATIVE_BIT = 0x8000;
static const uint16_t ENV_BITDEPTH = 14;

static const uint16_t SINLOG_BITDEPTH = 10;
static const uint16_t SINLOG_TABLESIZE = 1<<SINLOG_BITDEPTH;
extern const uint16_t sinLogTable[];

static const uint16_t SINEXP_BITDEPTH = 10;
static const uint16_t SINEXP_TABLESIZE = 1<<SINEXP_BITDEPTH;
extern const uint16_t sinExpTable[];

const uint16_t ENV_MAX = 1<<ENV_BITDEPTH;

const uint16_t sinLogTable[] = {
10597,8974,8219,7722,7351,7054,6808,6596,6411,6247,
6099,5965,5842,5728,5622,5524,5432,5345,5263,5185,
5111,5041,4974,4909,4848,4789,4732,4677,4624,4574,
4524,4477,4431,4386,4342,4300,4259,4219,4181,4143,
4106,4070,4035,4000,3967,3934,3902,3871,3840,3810,
3780,3751,3723,3695,3668,3641,3615,3589,3564,3539,
3514,3490,3466,3443,3420,3397,3375,3353,3331,3310,
3289,3268,3248,3228,3208,3188,3169,3150,3131,3112,
3094,3076,3058,3040,3023,3005,2988,2971,2955,2938,
2922,2906,2890,2874,2858,2843,2828,2812,2798,2783,
2768,2754,2739,2725,2711,2697,2683,2669,2656,2642,
2629,2616,2603,2590,2577,2564,2552,2539,2527,2515,
2502,2490,2478,2467,2455,2443,2432,2420,2409,2397,
2386,2375,2364,2353,2342,2331,2321,2310,2300,2289,
2279,2268,2258,2248,2238,2228,2218,2208,2198,2188,
2179,2169,2160,2150,2141,2131,2122,2113,2104,2095,
2086,2077,2068,2059,2050,2041,2032,2024,2015,2007,
1998,1990,1981,1973,1965,1957,1948,1940,1932,1924,
1916,1908,1900,1892,1885,1877,1869,1861,1854,1846,
1839,1831,1824,1816,1809,1801,1794,1787,1780,1772,
1765,1758,1751,1744,1737,1730,1723,1716,1709,1703,
1696,1689,1682,1676,1669,1662,1656,1649,1643,1636,
1630,1623,1617,1610,1604,1598,1592,1585,1579,1573,
1567,1561,1555,1548,1542,1536,1530,1525,1519,1513,
1507,1501,1495,1489,1484,1478,1472,1466,1461,1455,
1449,1444,1438,1433,1427,1422,1416,1411,1405,1400,
1395,1389,1384,1379,1373,1368,1363,1358,1352,1347,
1342,1337,1332,1327,1322,1317,1312,1307,1302,1297,
1292,1287,1282,1277,1272,1267,1262,1258,1253,1248,
1243,1239,1234,1229,1224,1220,1215,1211,1206,1201,
1197,1192,1188,1183,1179,1174,1170,1165,1161,1156,
1152,1148,1143,1139,1135,1130,1126,1122,1117,1113,
1109,1105,1100,1096,1092,1088,1084,1080,1076,1071,
1067,1063,1059,1055,1051,1047,1043,1039,1035,1031,
1027,1023,1019,1016,1012,1008,1004,1000,996,992,
989,985,981,977,973,970,966,962,959,955,
951,948,944,940,937,933,929,926,922,919,
915,912,908,905,901,898,894,891,887,884,
880,877,873,870,867,863,860,857,853,850,
847,843,840,837,833,830,827,824,820,817,
814,811,807,804,801,798,795,792,788,785,
782,779,776,773,770,767,764,761,758,755,
752,749,746,743,740,737,734,731,728,725,
722,719,716,713,710,708,705,702,699,696,
693,690,688,685,682,679,676,674,671,668,
665,663,660,657,655,652,649,646,644,641,
638,636,633,631,628,625,623,620,617,615,
612,610,607,605,602,600,597,594,592,589,
587,584,582,579,577,575,572,570,567,565,
562,560,558,555,553,550,548,546,543,541,
539,536,534,532,529,527,525,522,520,518,
515,513,511,509,506,504,502,500,497,495,
493,491,489,486,484,482,480,478,476,473,
471,469,467,465,463,461,459,456,454,452,
450,448,446,444,442,440,438,436,434,432,
430,428,426,424,422,420,418,416,414,412,
410,408,406,404,402,400,398,396,394,393,
391,389,387,385,383,381,379,378,376,374,
372,370,368,367,365,363,361,359,358,356,
354,352,350,349,347,345,343,342,340,338,
337,335,333,331,330,328,326,325,323,321,
320,318,316,315,313,311,310,308,306,305,
303,302,300,298,297,295,294,292,290,289,
287,286,284,283,281,280,278,276,275,273,
272,270,269,267,266,264,263,261,260,258,
257,256,254,253,251,250,248,247,245,244,
243,241,240,238,237,236,234,233,232,230,
229,227,226,225,223,222,221,219,218,217,
215,214,213,211,210,209,208,206,205,204,
202,201,200,199,197,196,195,194,192,191,
190,189,187,186,185,184,183,181,180,179,
178,177,175,174,173,172,171,170,169,167,
166,165,164,163,162,161,159,158,157,156,
155,154,153,152,151,150,149,148,146,145,
144,143,142,141,140,139,138,137,136,135,
134,133,132,131,130,129,128,127,126,125,
124,123,122,121,120,119,118,117,116,116,
115,114,113,112,111,110,109,108,107,106,
106,105,104,103,102,101,100,99,99,98,
97,96,95,94,94,93,92,91,90,89,
89,88,87,86,85,85,84,83,82,81,
81,80,79,78,78,77,76,75,75,74,
73,72,72,71,70,70,69,68,67,67,
66,65,65,64,63,63,62,61,61,60,
59,59,58,57,57,56,55,55,54,54,
53,52,52,51,51,50,49,49,48,48,
47,46,46,45,45,44,44,43,42,42,
41,41,40,40,39,39,38,38,37,37,
36,36,35,35,34,34,33,33,32,32,
31,31,30,30,29,29,28,28,28,27,
27,26,26,25,25,25,24,24,23,23,
23,22,22,21,21,21,20,20,19,19,
19,18,18,18,17,17,17,16,16,16,
15,15,15,14,14,14,13,13,13,12,
12,12,12,11,11,11,10,10,10,10,
9,9,9,9,8,8,8,8,7,7,
7,7,7,6,6,6,6,6,5,5,
5,5,5,4,4,4,4,4,4,3,
3,3,3,3,3,3,2,2,2,2,
2,2,2,2,2,1,1,1,1,1,
1,1,1,1,1,1,1,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0};
const uint16_t sinExpTable[] = {
0,3,6,8,11,14,17,19,22,25,
28,31,33,36,39,42,45,47,50,53,
56,59,61,64,67,70,73,76,78,81,
84,87,90,93,95,98,101,104,107,110,
112,115,118,121,124,127,130,132,135,138,
141,144,147,150,152,155,158,161,164,167,
170,173,176,178,181,184,187,190,193,196,
199,202,205,207,210,213,216,219,222,225,
228,231,234,237,240,243,246,248,251,254,
257,260,263,266,269,272,275,278,281,284,
287,290,293,296,299,302,305,308,311,314,
317,320,323,326,329,332,335,338,341,344,
347,350,353,356,359,362,365,368,371,374,
377,380,383,386,389,392,395,398,401,404,
407,410,413,416,419,422,425,429,432,435,
438,441,444,447,450,453,456,459,462,465,
469,472,475,478,481,484,487,490,493,496,
500,503,506,509,512,515,518,521,524,528,
531,534,537,540,543,546,550,553,556,559,
562,565,568,572,575,578,581,584,587,591,
594,597,600,603,607,610,613,616,619,622,
626,629,632,635,638,642,645,648,651,655,
658,661,664,667,671,674,677,680,684,687,
690,693,696,700,703,706,709,713,716,719,
723,726,729,732,736,739,742,745,749,752,
755,759,762,765,768,772,775,778,782,785,
788,792,795,798,801,805,808,811,815,818,
821,825,828,831,835,838,841,845,848,851,
855,858,861,865,868,872,875,878,882,885,
888,892,895,899,902,905,909,912,915,919,
922,926,929,932,936,939,943,946,949,953,
956,960,963,967,970,973,977,980,984,987,
991,994,998,1001,1004,1008,1011,1015,1018,1022,
1025,1029,1032,1036,1039,1043,1046,1050,1053,1056,
1060,1063,1067,1070,1074,1077,1081,1084,1088,1091,
1095,1099,1102,1106,1109,1113,1116,1120,1123,1127,
1130,1134,1137,1141,1144,1148,1152,1155,1159,1162,
1166,1169,1173,1176,1180,1184,1187,1191,1194,1198,
1201,1205,1209,1212,1216,1219,1223,1227,1230,1234,
1237,1241,1245,1248,1252,1256,1259,1263,1266,1270,
1274,1277,1281,1285,1288,1292,1296,1299,1303,1307,
1310,1314,1317,1321,1325,1328,1332,1336,1340,1343,
1347,1351,1354,1358,1362,1365,1369,1373,1376,1380,
1384,1388,1391,1395,1399,1402,1406,1410,1414,1417,
1421,1425,1429,1432,1436,1440,1444,1447,1451,1455,
1459,1462,1466,1470,1474,1477,1481,1485,1489,1492,
1496,1500,1504,1508,1511,1515,1519,1523,1527,1530,
1534,1538,1542,1546,1550,1553,1557,1561,1565,1569,
1572,1576,1580,1584,1588,1592,1596,1599,1603,1607,
1611,1615,1619,1623,1626,1630,1634,1638,1642,1646,
1650,1654,1658,1661,1665,1669,1673,1677,1681,1685,
1689,1693,1697,1701,1704,1708,1712,1716,1720,1724,
1728,1732,1736,1740,1744,1748,1752,1756,1760,1764,
1768,1772,1776,1780,1784,1788,1791,1795,1799,1803,
1807,1811,1815,1819,1823,1827,1831,1835,1840,1844,
1848,1852,1856,1860,1864,1868,1872,1876,1880,1884,
1888,1892,1896,1900,1904,1908,1912,1916,1920,1924,
1929,1933,1937,1941,1945,1949,1953,1957,1961,1965,
1969,1974,1978,1982,1986,1990,1994,1998,2002,2007,
2011,2015,2019,2023,2027,2031,2036,2040,2044,2048,
2052,2056,2060,2065,2069,2073,2077,2081,2086,2090,
2094,2098,2102,2106,2111,2115,2119,2123,2128,2132,
2136,2140,2144,2149,2153,2157,2161,2166,2170,2174,
2178,2183,2187,2191,2195,2200,2204,2208,2212,2217,
2221,2225,2229,2234,2238,2242,2247,2251,2255,2259,
2264,2268,2272,2277,2281,2285,2290,2294,2298,2303,
2307,2311,2316,2320,2324,2329,2333,2337,2342,2346,
2350,2355,2359,2364,2368,2372,2377,2381,2385,2390,
2394,2399,2403,2407,2412,2416,2421,2425,2430,2434,
2438,2443,2447,2452,2456,2461,2465,2469,2474,2478,
2483,2487,2492,2496,2501,2505,2510,2514,2518,2523,
2527,2532,2536,2541,2545,2550,2554,2559,2563,2568,
2572,2577,2581,2586,2590,2595,2600,2604,2609,2613,
2618,2622,2627,2631,2636,2640,2645,2650,2654,2659,
2663,2668,2672,2677,2682,2686,2691,2695,2700,2705,
2709,2714,2718,2723,2728,2732,2737,2742,2746,2751,
2755,2760,2765,2769,2774,2779,2783,2788,2793,2797,
2802,2807,2811,2816,2821,2825,2830,2835,2839,2844,
2849,2854,2858,2863,2868,2872,2877,2882,2887,2891,
2896,2901,2905,2910,2915,2920,2924,2929,2934,2939,
2943,2948,2953,2958,2963,2967,2972,2977,2982,2986,
2991,2996,3001,3006,3010,3015,3020,3025,3030,3035,
3039,3044,3049,3054,3059,3064,3068,3073,3078,3083,
3088,3093,3098,3102,3107,3112,3117,3122,3127,3132,
3137,3142,3146,3151,3156,3161,3166,3171,3176,3181,
3186,3191,3196,3201,3206,3210,3215,3220,3225,3230,
3235,3240,3245,3250,3255,3260,3265,3270,3275,3280,
3285,3290,3295,3300,3305,3310,3315,3320,3325,3330,
3335,3340,3345,3350,3355,3360,3365,3370,3376,3381,
3386,3391,3396,3401,3406,3411,3416,3421,3426,3431,
3436,3442,3447,3452,3457,3462,3467,3472,3477,3482,
3488,3493,3498,3503,3508,3513,3518,3524,3529,3534,
3539,3544,3549,3555,3560,3565,3570,3575,3581,3586,
3591,3596,3601,3607,3612,3617,3622,3628,3633,3638,
3643,3648,3654,3659,3664,3669,3675,3680,3685,3690,
3696,3701,3706,3712,3717,3722,3727,3733,3738,3743,
3749,3754,3759,3765,3770,3775,3781,3786,3791,3797,
3802,3807,3813,3818,3823,3829,3834,3839,3845,3850,
3856,3861,3866,3872,3877,3883,3888,3893,3899,3904,
3910,3915,3920,3926,3931,3937,3942,3948,3953,3959,
3964,3969,3975,3980,3986,3991,3997,4002,4008,4013,
4019,4024,4030,4035,4041,4046,4052,4057,4063,4068,
4074,4079,4085,4090};


static inline uint16_t sinLog(uint16_t phi) {
    const uint16_t SINLOG_TABLEFILTER = SINLOG_TABLESIZE-1;
    const uint16_t index = (phi & SINLOG_TABLEFILTER);
    
    switch( ( phi & (SINLOG_TABLESIZE * 3) ) ) {
        case 0:
            return sinLogTable[index];
        case SINLOG_TABLESIZE:
            return sinLogTable[index ^ SINLOG_TABLEFILTER];
        case SINLOG_TABLESIZE * 2 :
            return sinLogTable[index] | NEGATIVE_BIT;
        default:
            return sinLogTable[index ^ SINLOG_TABLEFILTER] | NEGATIVE_BIT;
    }
}

EngineMkI::EngineMkI() {
    
#ifdef MKIDEBUG
    char buffer[4096];
    int pos = 0;
    
    TRACE("****************************************");
    for(int i=0;i<SINLOG_TABLESIZE;i++) {
        pos += sprintf(buffer+pos, "%d ", sinLogTable[i]);
        if ( pos > 90 ) {
            TRACE("SINLOGTABLE: %s" ,buffer);
            buffer[0] = 0;
            pos = 0;
        }
    }
    TRACE("SINLOGTABLE: %s", buffer);
    buffer[0] = 0;
    pos = 0;
    TRACE("----------------------------------------");    
    for(int i=0;i<SINEXP_TABLESIZE;i++) {
        pos += sprintf(buffer+pos, "%d ", sinExpTable[i]);
        if ( pos > 90 ) {
            TRACE("SINEXTTABLE: %s" ,buffer);
            buffer[0] = 0;
            pos = 0;
        }
    }
    TRACE("SINEXTTABLE: %s", buffer);
    TRACE("****************************************");
#endif
}

inline int32_t mkiSin(int32_t phase, uint16_t env) {
    uint16_t expVal = sinLog(phase >> (22 - SINLOG_BITDEPTH)) + (env);
    //int16_t expValShow = expVal;
    
    const bool isSigned = expVal & NEGATIVE_BIT;
    expVal &= ~NEGATIVE_BIT;
    
    const uint16_t SINEXP_FILTER = 0x3FF;
    uint16_t result = 4096 + sinExpTable[( expVal & SINEXP_FILTER ) ^ SINEXP_FILTER];
    
    //uint16_t resultB4 = result;
    result >>= ( expVal >> 10 ); // exp
    
#ifdef MKIDEBUG
    if ( ( time(NULL) % 5 ) == 0 ) {
        if ( expValShow < 0 ) {
            expValShow = (expValShow + 0x7FFF) * -1;
        }
        //TRACE(",%d,%d,%d,%d,%d,%d", phase >> (22 - SINLOG_BITDEPTH), env, expValShow, ( expVal & SINEXP_FILTER ) ^ SINEXP_FILTER, resultB4, result);
    }
#endif
    
    if( isSigned )
        return (-result - 1) << 13;
    else
        return result << 13;
}

void EngineMkI::compute(int32_t *output, const int32_t *input,
                        int32_t phase0, int32_t freq,
                        int32_t gain1, int32_t gain2, bool add) {
    int32_t dgain = (gain2 - gain1 + (N >> 1)) >> LG_N;
    int32_t gain = gain1;
    int32_t phase = phase0;
    const int32_t *adder = add ? output : zeros;
    
    for (int i = 0; i < N; i++) {
        gain += dgain;
        int32_t y = mkiSin((phase+input[i]), gain);
        output[i] = y + adder[i];
        phase += freq;
    }
    
}

void EngineMkI::compute_pure(int32_t *output, int32_t phase0, int32_t freq,
                             int32_t gain1, int32_t gain2, bool add) {
    int32_t dgain = (gain2 - gain1 + (N >> 1)) >> LG_N;
    int32_t gain = gain1;
    int32_t phase = phase0;
    const int32_t *adder = add ? output : zeros;
    
    for (int i = 0; i < N; i++) {
        gain += dgain;
        int32_t y = mkiSin(phase , gain);
        output[i] = y + adder[i];
        phase += freq;
    }
}

void EngineMkI::compute_fb(int32_t *output, int32_t phase0, int32_t freq,
                           int32_t gain1, int32_t gain2,
                           int32_t *fb_buf, int fb_shift, bool add) {
    int32_t dgain = (gain2 - gain1 + (N >> 1)) >> LG_N;
    int32_t gain = gain1;
    int32_t phase = phase0;
    const int32_t *adder = add ? output : zeros;
    int32_t y0 = fb_buf[0];
    int32_t y = fb_buf[1];
    for (int i = 0; i < N; i++) {
        gain += dgain;
        int32_t scaled_fb = (y0 + y) >> (fb_shift + 1);
        y0 = y;
        y = mkiSin((phase+scaled_fb), gain);
        output[i] = y + adder[i];
        phase += freq;
    }
    
    fb_buf[0] = y0;
    fb_buf[1] = y;
}

// exclusively used for ALGO 6 with feedback
void EngineMkI::compute_fb2(int32_t *output, FmOpParams *parms, int32_t gain01, int32_t gain02, int32_t *fb_buf, int fb_shift) {
    int32_t dgain[2];
    int32_t gain[2];
    int32_t phase[2];
    int32_t y0 = fb_buf[0];
    int32_t y = fb_buf[1];

    phase[0] = parms[0].phase;
    phase[1] = parms[1].phase;

    parms[1].gain_out = (ENV_MAX-(parms[1].level_in >> (28-ENV_BITDEPTH)));

    gain[0] = gain01;
    gain[1] = parms[1].gain_out == 0 ? (ENV_MAX-1) : parms[1].gain_out;

    dgain[0] = (gain02 - gain01 + (N >> 1)) >> LG_N;
    dgain[1] = (parms[1].gain_out - (parms[1].gain_out == 0 ? (ENV_MAX-1) : parms[1].gain_out));
    
    for (int i = 0; i < N; i++) {
        int32_t scaled_fb = (y0 + y) >> (fb_shift + 1);
        
        // op 0
        gain[0] += dgain[0];
        y0 = y;
        y = mkiSin(phase[0]+scaled_fb, gain[0]);
        phase[0] += parms[0].freq;
        
        // op 1
        gain[1] += dgain[1];
        y = mkiSin(phase[1]+y, gain[1]);
        phase[1] += parms[1].freq;
        
        output[i] = y;
    }
    fb_buf[0] = y0;
    fb_buf[1] = y;
}

// exclusively used for ALGO 4 with feedback
void EngineMkI::compute_fb3(int32_t *output, FmOpParams *parms, int32_t gain01, int32_t gain02, int32_t *fb_buf, int fb_shift) {
    int32_t dgain[3];
    int32_t gain[3];
    int32_t phase[3];
    int32_t y0 = fb_buf[0];
    int32_t y = fb_buf[1];
    
    phase[0] = parms[0].phase;
    phase[1] = parms[1].phase;
    phase[2] = parms[2].phase;
    
    parms[1].gain_out = (ENV_MAX-(parms[1].level_in >> (28-ENV_BITDEPTH)));
    parms[2].gain_out = (ENV_MAX-(parms[2].level_in >> (28-ENV_BITDEPTH)));
    
    gain[0] = gain01;
    gain[1] = parms[1].gain_out == 0 ? (ENV_MAX-1) : parms[1].gain_out;
    gain[2] = parms[2].gain_out == 0 ? (ENV_MAX-1) : parms[2].gain_out;

    dgain[0] = (gain02 - gain01 + (N >> 1)) >> LG_N;
    dgain[1] = (parms[1].gain_out - (parms[1].gain_out == 0 ? (ENV_MAX-1) : parms[1].gain_out));
    dgain[2] = (parms[2].gain_out - (parms[2].gain_out == 0 ? (ENV_MAX-1) : parms[2].gain_out));
    
    
    for (int i = 0; i < N; i++) {
        int32_t scaled_fb = (y0 + y) >> (fb_shift + 1);
        
        // op 0
        gain[0] += dgain[0];
        y0 = y;
        y = mkiSin(phase[0]+scaled_fb, gain[0]);
        phase[0] += parms[0].freq;
        
        // op 1
        gain[1] += dgain[1];
        y = mkiSin(phase[1]+y, gain[1]);
        phase[1] += parms[1].freq;
        
        // op 2
        gain[2] += dgain[2];
        y = mkiSin(phase[2]+y, gain[2]);
        phase[2] += parms[2].freq;
        
        output[i] = y;
    }
    fb_buf[0] = y0;
    fb_buf[1] = y;
}

void EngineMkI::render(int32_t *output, FmOpParams *params, int algorithm, int32_t *fb_buf, int feedback_shift) {
    const int kLevelThresh = ENV_MAX-100;
    FmAlgorithm alg = algorithms[algorithm];
    bool has_contents[3] = { true, false, false };
    bool fb_on = feedback_shift < 16;

    switch(algorithm) {
        case 3 : case 5 :
            if ( fb_on )
                alg.ops[0] = 0xc4;
    }
    
    for (int op = 0; op < 6; op++) {
        int flags = alg.ops[op];
        bool add = (flags & OUT_BUS_ADD) != 0;
        FmOpParams &param = params[op];
        int inbus = (flags >> 4) & 3;
        int outbus = flags & 3;
        int32_t *outptr = (outbus == 0) ? output : &buf_[outbus - 1][0];
        int32_t gain1 = param.gain_out == 0 ? (ENV_MAX-1) : param.gain_out;
        int32_t gain2 = ENV_MAX-(param.level_in >> (28-ENV_BITDEPTH));
        param.gain_out = gain2;
        
        if (gain1 <= kLevelThresh || gain2 <= kLevelThresh) {
            
            if (!has_contents[outbus]) {
                add = false;
            }
            
            if (inbus == 0 || !has_contents[inbus]) {
                // PG: this is my 'dirty' implementation of FB for 2 and 3 operators...
                if ((flags & 0xc0) == 0xc0 && fb_on) {
                    switch ( algorithm ) {
                        // three operator feedback, process exception for ALGO 4
                        case 3 :
                            compute_fb3(outptr, params, gain1, gain2, fb_buf, min((feedback_shift+2), 16));
                            params[1].phase += params[1].freq << LG_N; // hack, we already processed op-5 - op-4
                            params[2].phase += params[2].freq << LG_N; // yuk yuk
                            op += 2; // ignore the 2 other operators
                            break;
                        // two operator feedback, process exception for ALGO 6
                        case 5 :
                            compute_fb2(outptr, params, gain1, gain2, fb_buf, min((feedback_shift+2), 16));
                            params[1].phase += params[1].freq << LG_N;  // yuk, hack, we already processed op-5
                            op++; // ignore next operator;
                            break;
                        default:
                            // one operator feedback, normal proces
                            compute_fb(outptr, param.phase, param.freq, gain1, gain2, fb_buf, feedback_shift, add);
                            break;
                    }
                } else {
                    compute_pure(outptr, param.phase, param.freq, gain1, gain2, add);
                }
            } else {
                compute(outptr, &buf_[inbus - 1][0], param.phase, param.freq, gain1, gain2, add);
            }
            
            has_contents[outbus] = true;
        } else if (!add) {
            has_contents[outbus] = false;
        }
        param.phase += param.freq << LG_N;
    }
}

