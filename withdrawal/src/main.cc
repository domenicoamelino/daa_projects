/*
 * Empty C++ Application
 */

#include <stdio.h>
#include <stdlib.h>
#include "pairing_3.h"
#include "platform.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
#include "xtime_l.h"

/* Definitions */
#define GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID	/* GPIO device that LEDs are connected to */
#define LED 0x01									/* Initial LED value - XX0000XX */
#define LED_DELAY 50000000							/* Software delay length */
#define LED_CHANNEL 1								/* GPIO port for LEDs */
#define printf xil_printf							/* smaller, optimised printf */

XGpio Gpio;											/* GPIO Device driver instance */

#define MR_PAIRING_BN    // AES-128 or AES-192 security
#define AES_SECURITY 128


#ifndef MR_NOFULLWIDTH
Miracl precision(50,0);
#else
Miracl precision(50,MAXBASE);
#endif

char *primetext=(char *) "155315526351482395991155996351231807220169644828378937433223838972232518351958838087073321845624756550146945246003790108045940383194773439496051917019892370102341378990113959561895891019716873290512815434724157588460613638202017020672756091067223336194394910765309830876066246480156617492164140095427773547319";
char *text=(char *)"MIRACL - Best multi-precision library in the World!\n";
int main()
{
	//Inizializzazione della piattaforma
	init_platform();
    int ia,ib;
    int num_hash = 1000;
    XTime tStart, tEnd;
    time_t seed;
    Big a,b,p,q,phi,n,pa,pb,key,e,d,m,c,x,y,k,inv,t;
    Big primes[2],pm[2];
    ECn g,ea,eb;
    char hash_n[32];
    char id_vector[20];
    int i;
    sha256 sh,sh2;
    miracl *mip=&precision;
	cout<<"TICKET WITHDRAWAL TEST"<<endl;
	 mip->IOBASE=16;
	Big c_start = rand(256,2);

	cout<<"Valore = "<<c_start<<endl;
    int len=to_binary(c_start,32,hash_n,FALSE);
    printf("Stringa =");
    for(int i=0;i<32;i++) printf("%02x",hash_n[i]);
    cout<<endl;
    XTime_GetTime(&tStart);
    for(int j=0;j<num_hash;j++)
    {
    	shs256_init(&sh);
    	for (i=0;hash_n[i]!=0;i++) shs256_process(&sh,hash_n[i]);
    	shs256_hash(&sh,hash_n);
    }

    XTime_GetTime(&tEnd);
    printf("Valore hashato : ");
    for (i=0;i<32;i++) printf("%02x",(unsigned char)hash_n[i]);
    printf("\n");
    cout<<"HASH CHAIN COMPUTATION:= "<<1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000)<<"us."<<endl;


    // Adesso devo implementare...sempre in simulazione, la fase di withdrawal.
	Big  pi=primetext;
	Big ID;
	ID = rand(160,2);
	cout<<"VALORE ID = "<<ID<<endl;
	len=to_binary(ID,20,id_vector,FALSE);

	printf("Stringa ID=");
	 for(int i=0;i<20;i++) printf("%02x",id_vector[i]);
	//cout<<"Valore di ID: "<<ID<<endl;
	// 808482096439117605921939216063679095635925463883 value is a safe prime of 192 bits - it indicates group number g
	shs256_init(&sh2);
	for (i=0;id_vector[i]!=0;i++) shs256_process(&sh2,id_vector[i]);
	shs256_hash(&sh2,id_vector);

	printf("Valore hashato : ");
	for (i=0;i<20;i++) printf("%02x",(unsigned char)id_vector[i]);
	printf("\n");


	//MANCA LA CONCATENAZIONEEEE


	//INIZIO TESTING
	//cout << "Setup.....";
	XTime_GetTime(&tStart);
	/* RSA. Generate primes p & q. Use e=65537, and find d=1/e mod (p-1)(q-1) */

	    cout << "\nNow generating 512-bit random primes p and q" << endl;
	    for(;;)
	    {
	        p=rand(512,2);        // random 512 bit number
	        if (p%2==0) p+=1;
	        while (!prime(p)) p+=2;

	        q=rand(512,2);
	        if (q%2==0) q+=1;
	        while (!prime(q)) q+=2;

	        n=p*q;

	        e=65537;
	        phi=(p-1)*(q-1);
	        if (gcd(e,phi)!=1) continue;
	        d=inverse(e,phi);
	        break;
	    }
	    cout << p << endl;
	    cout << q << endl;
	    cout << "n = p.q = \n";
	    cout << n << endl;

	/* set up for chinese remainder thereom */

	//    primes[0]=p;
	//   primes[1]=q;

	//    Crt chinese(2,primes);

	    inv=inverse(p,q);   // precalculate this

	    mip->IOBASE=256;

	    cout << "Encrypting test string" << endl;
	    c=pow((Big)text,e,n);         // c=m^e mod n
	    mip->IOBASE=10;
	    cout << "Ciphertext= \n";
	    cout << c << endl;

	    cout << "Decrypting test string" << endl;

	    pm[0]=pow(c%p,d%(p-1),p);    /* get result mod p */
	    pm[1]=pow(c%q,d%(q-1),q);    /* get result mod q */

	    t=modmult(inv,pm[1]-pm[0],q);  // use CRT in simple way, as only 2 primes
	    m=t*p+pm[0];

	 //   m=chinese.eval(pm);    /* combine them using CRT */

	    mip->IOBASE=256;
	    cout << "Plaintext= \n";
	    cout << m << endl;

	XTime_GetTime(&tEnd);

	//XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x03);
	//cout <<"..done" <<endl;

    //cout<<"SETUP := Output took "<<2*(tEnd - tStart)<<" clock cycles."<<endl;
    cout<<"WITHDRAWAL:= "<<1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000)<<"us."<<endl;


	return 0;
}
