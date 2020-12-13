/*
 * acd.c
 * 
 * Copyright 2020 igor <igor@igor-chumak>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */


/*

T   R       ADC value = V*100
 
24	10,449	255
25	10	    250
26	9,571	245
27	9,164	239
28	8,775	234
29	8,405	228
30	8,052	223
31	7,716	218
32	7,396	213
33	7,09	207
34	6,798	202
35	6,52	197
36	6,255	192
37	6,002	188
38	5,76	183
39	5,529	178
40	5,309	173
41	5,098	169
42	4,897	164
43	4,704	160
44	4,521	156
45	4,345	151
46	4,177	147
47	4,016	143
48	3,863	139
49	3,716	135
50	3,588	132
51	3,44	128
52	3,311	124
53	3,188	121
54	3,069	117
55	2,956	114
56	2,848	111
57	2,744	108
58	2,644	105
59	2,548	102
60	2,457	99
61	2,369	96
62	2,284	93
63	2,204	90
64	2,126	88
65	2,051	85
66	1,98	83
67	1,911	80
68	1,845	78
69	1,782	76
70	1,721	73
71	1,663	71
72	1,606	69
73	1,552	67
74	1,5	    65
75	1,45	63
76	1,402	61
77	1,356	60
78	1,312	58
79	1,269	56
80	1,228	55
81	1,188	53
82	1,15	52
83	1,113	50
84	1,078	49
85	1,044	47
86	1,011	46
87	0,979	45
88	0,948	43
89	0,919	42
90	0,891	41
91	0,863	40
92	0,837	39
93	0,811	38
94	0,787	36
95	0,763	35
96	0,74	34
97	0,718	33
98	0,697	33
99	0,676	32
100	0,657	31
101	0,637	30
102	0,619	29
103	0,601	28
104	0,584	28
105	0,567	27
106	0,551	26
107	0,535	25
108	0,52	25
109	0,505	24
110	0,491	23
111	0,477	23
112	0,464	22
113	0,451	22
114	0,439	21
115	0,427	20
116	0,415	20
117	0,404	19
118	0,393	19
119	0,383	18
120	0,373	18
121	0,363	18
122	0,353	17
123	0,344	17
124	0,335	16
125	0,326	16

 
 */
 #include "adc.h"

uint8_t adc_measures_base24[] = { 255, 250, 245, 239, 234, 228, 223, 218, 213, 207, 202, 197, 192, 188, 183, 178, 173, 169, 164, 160, 156, 151, 147, 143, 139, 135, 132, 128, 124, 121, 117, 114, 111, 108, 105, 102, 99, 96, 93, 90, 88, 85, 83, 80, 78, 76, 73, 71, 69, 67, 65, 63, 61, 60, 58, 56, 55, 53, 52, 50, 49, 47, 46, 45, 43, 42, 41, 40, 39, 38, 36, 35, 34, 33, 33, 32, 31, 30, 29, 28, 28, 27, 26, 25, 25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 18, 17, 17, 16, 16};

uint8_t adc_to_t(uint8_t val)
{
   uint8_t min_idx = 0;
   uint8_t max_idx = sizeof(adc_measures_base24)/sizeof(adc_measures_base24[0])-1;	
   uint8_t now_idx;
   now_idx = (min_idx + max_idx) / 2;
   while (val != adc_measures_base24[now_idx])
   {
   //printf("%d: %d .. %d .. %d\n", val, min_idx, now_idx, max_idx);
   if (now_idx == min_idx || now_idx == max_idx) break;
   if (val > adc_measures_base24[now_idx])
		{
			max_idx = now_idx;
		};
   if (val < adc_measures_base24[now_idx])
		{
			min_idx = now_idx;
		};
       now_idx = (min_idx + max_idx) / 2;
	};
	 return now_idx+24;
};

uint8_t ADC_read(uint8_t channel)
{
	ADMUX &= ~7;
	ADMUX |= (channel & 7);
	ADCSRA |= (1<<ADSC); //Начинаем преобразование
	while((ADCSRA & (1<<ADSC))); //проверим закончилось ли аналого-цифровое преобразование
	return ADCH; 
}
                      
