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

#define MR_PAIRING_MNT	// AES-80 security
#define AES_SECURITY 80


char *primetext=(char *)
"155315526351482395991155996351231807220169644828378937433223838972232518351958838087073321845624756550146945246003790108045940383194773439496051917019892370102341378990113959561895891019716873290512815434724157588460613638202017020672756091067223336194394910765309830876066246480156617492164140095427773547319";


int main()
{
	//Inizializzazione della piattaforma
	init_platform();

	cout<<"DAA implementation using MTN Curves (k=6)"<<endl;

	//Dati utili al test temporale
	 XTime tStart, tEnd, tBuff;

	//INIZIO TESTING
	cout << "Setup.....";
	XTime_GetTime(&tStart);
	PFC pfc(AES_SECURITY);
	XTime seed;

	int i,j;
	G1 g1,h1,h2,B;
	G2 g2,w;
	GT t1,t2,t3,t4;
	Big gamma;

//	XTime_GetTime(&seed);
 //   irand((long)seed);


	Big order=pfc.order();

	//cout<<"Ordine dell'insieme random da cui si prendono i valori --->"<<order<<endl;

	pfc.random(g1);
	//cout<<"Output G1 ="<<g1.g<<endl;
	//cout<<"PFC MOD = "<<*(pfc.mod)<<endl;
	pfc.random(g2);
	//cout<<"Output G2 = "<<g2.g<<endl;
	pfc.random(gamma);
	pfc.random(h1);
	pfc.random(h2);
	w=pfc.mult(g2,gamma);

	//XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x01);


	t1=pfc.pairing(g2,g1);  // ^t(P1,Q)
	t2=pfc.pairing(g2,h1);  // ^t(P2,Q)
	t3=pfc.pairing(g2,h2);  // ^t(P3,Q)
	t4=pfc.pairing(w,h2);   // ^t(P3,X)


	pfc.precomp_for_mult(g1);
	pfc.precomp_for_mult(g2);
	pfc.precomp_for_mult(h1);
	pfc.precomp_for_mult(h2);
	pfc.precomp_for_mult(w);
	pfc.precomp_for_pairing(g2);
	pfc.precomp_for_power(t1);
	pfc.precomp_for_power(t2);
	pfc.precomp_for_power(t3);
	pfc.precomp_for_power(t4);


	//XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x3F);

	XTime_GetTime(&tEnd);

	//XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x03);
	cout <<"..done" <<endl;

    //cout<<"SETUP := Output took "<<2*(tEnd - tStart)<<" clock cycles."<<endl;
    cout<<"SETUP := Output took "<<1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000)<<"us."<<endl;

	cout << "Join...";
	XTime_GetTime(&tStart);
	Big ni,f,sk,rf,c,sf,x,ci;
	G1 F,R;
	Big bohx,bohy,bohz;

	// TPM
	pfc.random(f);   // should be H(DAASeed||cnt||KI), i do not modify this step for now.
	sk=f;  // <-- this is the secret.
	pfc.random(rf);  // rf = u randomly choosed from Zp
	F=pfc.mult(h1,f);  // F <- h1^f
	R=pfc.mult(h1,rf); // this is U <- P2^u
	// The TPM now starts the hash calculation.
	// H = (X || P1 || P2 || P3 || Q || ni || F || U) il calcolo che viene eseguito in questi passaggi.
	pfc.start_hash();
	pfc.add_to_hash(order); pfc.add_to_hash(g1); pfc.add_to_hash(h1), pfc.add_to_hash(h2); pfc.add_to_hash(g2); pfc.add_to_hash(w);
	pfc.add_to_hash(ni); pfc.add_to_hash(F); pfc.add_to_hash(R);
	c=pfc.finish_hash_to_group(); // the result of the hash calculation
	sf=(rf+modmult(c,f,order))%order; // sf = w = u + f * v (mod p)
	//HOST
	G2 wxg2=w+pfc.mult(g2,x);
	G1 g1f=-(F+g1);

		XTime_GetTime(&tEnd);
	tBuff = tEnd - tStart;
	// MADE FROM THE ISSUER....NON LO CONSIDERO, lo inserisco poi come STUB..
	G1 A=pfc.mult(g1+F,inverse(x+gamma,order));

	XTime_GetTime(&tStart);

	//HOST
	G1 *gf1[2];
	G2 *gf2[2];
	gf1[0]=&A;
	gf1[1]=&g1f;
	gf2[0]=&wxg2;
	gf2[1]=&g2;
	if (pfc.multi_pairing(2,gf2,gf1)!=1)  // <-- questa funzione l'ho copiata, fa il confronto tra due funzioni di pairing!!
	{
		cout << "Verification fails, aborting.. " << endl;
		exit(0);
	}

	XTime_GetTime(&tEnd);

	cout<<"....done"<<endl;
	//XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1F);

    // cout<<"JOIN := Output took "<<2*(tEnd - tStart)<<" clock cycles."<<endl;
    cout<<"JOIN := Output took "<<1.0 * ((tEnd - tStart)+tBuff) / (COUNTS_PER_SECOND/1000000)<<"us."<<endl;

	cout << "Sign......"<<endl;

	XTime_GetTime(&tStart);
	G1 K,R1,R2t,nv;
	//TPM
	pfc.hash_and_map(B,(char *)"bsn");
	//cout<<"Hash and Map Fatta"<<endl;
	pfc.random(rf); //picks a random value rf from Zp group
	//cout<<"Prelievo random rf fatto";

	K=pfc.mult(B,f); // K = J^f
	//cout<<"Mult del cazzo..fatta"<<endl;
	R1=pfc.mult(B,rf); // called L into paper R1 = J^rf
	R2t=pfc.mult(h1,rf);

	//X value for Remote Activation Method:
	//  g ^ ID (mod p)
	//  p = primenumber 1024 bit
	//  g group identificatore = 160 bit taken from number value
	//Big  group_value = safeprime;

	Big  p=primetext;
	Big ID;
	ID = rand(160,2);
	//cout<<"Valore di ID: "<<ID<<endl;
	// 808482096439117605921939216063679095635925463883 value is a safe prime of 192 bits - it indicates group number g
	Big gen_X = pow(8084820964391176059219392160636790956359,ID,p);
	//cout<<"Valore X da inviare al verifier insieme al testo = "<<gen_X<<endl;


	// TPM sends B, K, R1, R2t to Host
	//HOST
	G1 T;
	GT R2;
	Big a,b,rx,ra,rb,ch,nt;
	pfc.random(a); //from Zp (Zp is the a group with big number probably order "order")).

	b=modmult(a,x,order);

	T=A+pfc.mult(h2,a);

	pfc.random(rx);
	pfc.random(ra);
	pfc.random(rb);



	R2=pfc.pairing(g2,R2t+pfc.mult(T,-rx)+pfc.mult(h2,rb))*pfc.power(t4,ra);
	pfc.start_hash();
	pfc.add_to_hash(order); pfc.add_to_hash(g1); pfc.add_to_hash(h1), pfc.add_to_hash(h2); pfc.add_to_hash(g2); pfc.add_to_hash(w);
	pfc.add_to_hash(B); pfc.add_to_hash(K); pfc.add_to_hash(T); pfc.add_to_hash(R1); pfc.add_to_hash(R2); pfc.add_to_hash(nv);
	ch=pfc.finish_hash_to_group();


	//HOST sends ch to TPM

	// TPM does..
	pfc.random(nt); // generates a random nt
	pfc.start_hash(); pfc.add_to_hash(ch); pfc.add_to_hash(nt); pfc.add_to_hash(gen_X);
	//pfc.add_to_hash(gen_X);
	c=pfc.finish_hash_to_group();
	sf=(rf+modmult(c,f,order))%order;
// {c,nt,sf) sent to Host
	rf=0; // rf is erased. rf is erased in order to prevent REPLAY ATTACKS.

	//HOST
	Big sx,sa,sb;
	sx=(rx+modmult(c,x,order))%order;
	sa=(ra+modmult(c,a,order))%order;
	sb=(rb+modmult(c,b,order))%order;

	XTime_GetTime(&tEnd);

	cout<<"....done."<<endl;
    cout<<"SIGN := Output took "<<1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000)<<"us."<<endl;

	return 0;
}
