//
// Created by matth on 2/18/2020.
//

#ifndef IOTA_SIMPLEWALLET_GENERATE_SEED_H
#define IOTA_SIMPLEWALLET_GENERATE_SEED_H
#include <stdint.h>
//Generates an IOTA seed
void generate_seed(char* buffer, uint32_t buf_max_len);

//Zero an IOTA seed ensuring compiler doesn't optimize out
void destroy_seed(char* buffer);
#endif //IOTA_SIMPLEWALLET_GENERATE_SEED_H




/*
//iota - randomness seed generator test

a 30001507
b 30002589
c 29998688
d 30005067
e 30000231
f 29993059
g 29999885
h 30002275
i 30002084
j 29997566
k 30004191
l 30000551
m 30003933
n 29999727
o 29998955
p 30012952
q 29996511
r 30004130
s 29994252
t 30002979
u 29989180
v 30000377
w 30000085
x 30012158
y 29988774
z 29992876
9 29995418

int iterator;

int nine = 0,
  a = 0,
  b=0,
  c=0,
  d=0,
  e=0,
  f=0,
  g=0,
  h=0,
  i=0,
  j=0,
  k=0,
  l=0,
  m=0,
  n=0,
  o=0,
  p=0,
  q=0,
  r=0,
  s=0,
  t=0,
  u=0,
  v=0,
  w=0,
  x=0,
  y=0,
  z=0;

for(iterator=0; iterator<10000000; iterator++) {
char seed[128];
generate_seed(seed, 128);
// printf("%s -- %d\n", seed, (int)strlen(seed));
if(iterator % 1000 == 0) {
printf("%d\n", iterator);
}
int count =  0;
for(count=0; count < 81; count++) {
char cha = seed[count];
switch(cha) {
case 'A':
a++; break;
case 'B':
b++; break;
case 'C':
c++; break;
case 'D':
d++; break;
case 'E':
e++; break;
case 'F':
f++; break;
case 'G':
g++; break;
case 'H':
h++; break;
case 'I':
i++; break;
case 'J':
j++; break;
case 'K':
k++; break;
case 'L':
l++; break;
case 'M':
m++; break;
case 'N':
n++; break;
case 'O':
o++; break;
case 'P':
p++; break;
case 'Q':
q++; break;
case 'R':
r++; break;
case 'S':
s++; break;
case 'T':
t++; break;
case 'U':
u++; break;
case 'V':
v++; break;
case 'W':
w++; break;
case 'X':
x++; break;
case 'Y':
y++; break;
case 'Z':
z++; break;
case '9':
nine++; break;
default:
fprintf(stderr, "error!\n");
};
}



destroy_seed(seed);
}
printf("a %d\nb %d\nc %d\nd %d\ne %d\nf %d\ng %d\nh %d\ni %d\nj %d\nk %d\nl %d\nm %d\nn %d\no %d\np %d\nq %d\nr %d\ns %d\nt %d\nu %d\nv %d\nw %d\nx %d\ny %d\nz %d\n9 %d\n",
a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,nine);
*/