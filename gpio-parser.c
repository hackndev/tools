#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int gpio;
	int fn;
	char dir;
	char *desc;
} GPIO_info;

typedef struct {
	int gpio;
	char *desc;
} GPIO_info_pv;

const GPIO_info pxa261[] = {
	{ 1, 1, 'i', "Active low GP_reset"},
	{ 6, 1, 'o', "MMC Clock"},
	{ 7, 1, 'o', "48-MHz clock output"},
	{ 8, 1, 'o', "MMC Chip Select 0"},
	{ 9, 1, 'o', "MMC Chip Select 1"},
	{ 9, 1, 'i', "USB Device Controller RCV"},
	{10, 1, 'o', "real time clock (1 Hz)"},
	{11, 1, 'o', "3.6-MHz oscillator out"},
	{12, 1, 'o', "32-KHz out"},
	{13, 2, 'o', "memory controller grant"},
	{14, 1, 'i', "memory controller alternate bus master request"},
	{15, 2, 'o', "Active low chip select 1"},
	{16, 2, 'o', "PWM0 outpout"},
	{17, 2, 'o', "PWM1 output"},
	{18, 1, 'i', "Ext. Bus Ready"},
	{19, 1, 'i', "External DMA Request"},
	{20, 1, 'i', "External DMA Request"},
	{23, 2, 'o', "SSP clock"},
	{24, 2, 'o', "SSP Frame"},
	{25, 2, 'o', "SSP transmit"},
	{26, 1, 'i', "SSP receive"},
	{27, 1, 'i', "SSP ext_clk"},
	{28, 1, 'i', "AC97 bit_clk"},
	{28, 2, 'i', "I2S bit_clk"},
	{28, 1, 'o', "I2S bit_clk"},
	{28, 3, 'i', "ASSP serial clock"},
	{28, 3, 'o', "ASSP serial clock"},
	{29, 1, 'i', "AC97 Sdata_in0"},
	{29, 2, 'i', "I2S Sdata_in"},
	{29, 3, 'i', "ASSP receive data"},
	{30, 1, 'o', "I2S Sdata_out"},
	{30, 2, 'o', "AC97 Sdata_out"},
	{30, 3, 'o', "ASSP transmit data"},
	{31, 1, 'o', "I2S sync"},
	{31, 2, 'o', "AC97 sync"},
	{31, 1, 'i', "ASSP frame"},
	{31, 3, 'o', "ASSP frame"},
	{32, 1, 'i', "AC97 Sdata_in1"},
	{32, 1, 'o', "I2S System Clock"},
	{32, 2, 'i', "USB Device Controller VP"},
	{33, 2, 'o', "Active low chip select 5"},
	{34, 1, 'i', "FFUART receive"},
	{34, 2, 'o', "MMC Chip Select 0"},
	{34, 2, 'i', "USB Device Controller VM"},
	{35, 1, 'i', "FFUART Clear to send"},
	{36, 1, 'i', "FFUART Data carrier detect"},
	{37, 1, 'i', "FFUART data set ready"},
	{38, 1, 'i', "FFUART Ring Indicator"},
	{39, 1, 'o', "MMC Chip Select 1"},
	{39, 2, 'o', "FFUART transmit data"},
	{39, 3, 'o', "USB Device Controller VPO"},
	{40, 2, 'o', "FFUART data terminal Ready"},
	{41, 2, 'o', "FFUART request to send"},
	{42, 1, 'i', "BTUART receive data"},
	{42, 3, 'i', "HWUART receive data"},
	{43, 2, 'o', "BTUART transmit data"},
	{43, 3, 'o', "HWUART transmit data"},
	{44, 1, 'i', "BTUART clear to send"},
	{44, 3, 'i', "HWUART clear to send"},
	{45, 2, 'o', "BTUART request to send"},
	{45, 3, 'o', "HWUART request to send"},
	{46, 1, 'i', "ICP receive data"},
	{46, 2, 'i', "STD_UART receive data"},
	{47, 1, 'o', "STD_UART transmit data"},
	{47, 2, 'o', "ICP transmit data"},
	{48, 2, 'o', "Output Enable for Card Space"},
	{48, 1, 'o', "HWUART transmit"},
	{49, 2, 'o', "Write Enable for Card Space"},
	{49, 1, 'i', "HWUART receive"},
	{50, 2, 'o', "I/O Read for Card Space"},
	{50, 1, 'i', "HWUART Clear to send"},
	{51, 2, 'o', "I/O Write for Card Space"},
	{51, 1, 'o', "HWUART Request to Send"},
	{52, 2, 'o', "Card Enable for Card Space"},
	{53, 2, 'o', "Card Enable for Card Space"},
	{53, 1, 'o', "MMC Clock"},
	{54, 1, 'o', "MMC Clock"},
	{54, 2, 'o', "Socket Select for Card Space"},
	{55, 2, 'o', "Card Address bit 26"},
	{56, 1, 'i', "Wait signal for Card Space"},
	{56, 1, 'o', "USB Device Controller VMO"},
	{57, 1, 'i', "Bus Width select for I/O Card Space"},
	{57, 1, 'o', "USB Device Controller nOE"},
	{58, 2, 'o', "LCD data pin 0"},
	{59, 2, 'o', "LCD data pin 1"},
	{60, 2, 'o', "LCD data pin 2"},
	{61, 2, 'o', "LCD data pin 3"},
	{62, 2, 'o', "LCD data pin 4"},
	{63, 2, 'o', "LCD data pin 5"},
	{64, 2, 'o', "LCD data pin 6"},
	{65, 2, 'o', "LCD data pin 7"},
	{66, 2, 'o', "LCD data pin 8"},
	{66, 1, 'i', "memory controller alternate bus master req"},
	{67, 2, 'o', "LCD data pin 9"},
	{67, 1, 'o', "MMC Chip Select 0"},
	{68, 1, 'o', "MMC Chip Select 1"},
	{68, 2, 'o', "LCD data pin 10"},
	{69, 1, 'o', "MMC_CLK"},
	{69, 2, 'o', "LCD data pin 11"},
	{70, 1, 'o', "Real Time clock (1 Hz)"},
	{70, 2, 'o', "LCD data pin 12"},
	{71, 1, 'o', "3.6-MHz Oscillator clock"},
	{71, 2, 'o', "LCD data pin 13"},
	{72, 1, 'o', "32-KHz clock"},
	{72, 2, 'o', "LCD data pin 14"},
	{73, 2, 'o', "LCD data pin 15"},
	{73, 1, 'o', "Memory controller grant"},
	{74, 2, 'o', "LCD Frame clock"},
	{75, 2, 'o', "LCD line clock"},
	{76, 2, 'o', "LCD Pixel clock"},
	{77, 2, 'o', "LCD AC Bias"},
	{78, 2, 'o', "Active low chip select 2"},
	{79, 2, 'o', "Active low chip select 3"},
	{80, 2, 'o', "Active low chip select 4"},
	{81, 1, 'i', "NSSP Serial clock is input"},
	{81, 1, 'o', "NSSP Serial clock is output"},
	{82, 1, 'i', "NSSP frame is input"},
	{82, 1, 'o', "NSSP frame is output"},
	{83, 1, 'o', "NSSP transmit"},
	{83, 2, 'i', "NSSP receive"},
	{84, 1, 'o', "NSSP transmit"},
	{84, 2, 'i', "NSSP receive"},
	{86, 0, 'o', "SDRAM chip select 2"},
	{87, 0, 'o', "SDRAM chip select 3"},
	{88, 0, 'o', "Memory Controller Read/write"},
	{89, 0, 'o', "AC97 reset"},
	{-1, 0, '\n', "\n"}
};

const GPIO_info_pv tt3[] = {
	{0, "calendar/contacts/voice"},
	{2, "W86L488Y card insert"},
	{3, "slider"},
	{4, "charging (active low)"},
	{5, "cradle connected"},
	{10, "5nav up/down/left/right"},
	{11, "memo/todo/center"},
	{14, "power button (works only one time, weird"},
	{19, "hw buttons output"},
	{20, "hw buttons output"},
	{21, "hw buttons output"},
	{22, "hw buttons output"},
	{37, "TSC2101 screen touched"},
	{38, "// High, when set to low screen is whiteouted (TSC2101's second GPIO?)"},
	{53, "PUC serial mode if high (not confirmed)"},
	{85, "PUC power (active high)"},

	{-1, "\n"}
};
		
unsigned long int GPLR[3];
unsigned long int GPDR[3];
unsigned long int GRER[3];
unsigned long int GFER[3];
unsigned long int GEDR[3];
unsigned long int GAFR[6];

char *getGPIOstr(int id, int fn, char dir)
{
	GPIO_info *p = pxa261;
	GPIO_info_pv *pv = tt3;
	int found = 0;
	while(p->dir != '\n') {
		if(p->gpio == id && p->fn == fn && p->dir == dir) {
			found = 1;
			break;
		} else
			p++;
	}
	if(found) return p->desc;
	
	while(pv->gpio != -1) {
		if(pv->gpio == id) return pv->desc;
		pv++;
	}
	return "?";
}

void printGPIO(int id, int lr, int dr, int rer, int fer, int edr, int afr)
{
	int real_dr = (id<86) ? dr : !dr;
	
	printf("%2d: %s %s  %s %s  %d %s\n", id, lr?"*":" ", real_dr?"->":"<-",
		   rer?"_/":"  ", fer?"\\_":"  ", afr, getGPIOstr(id, afr, real_dr?'o':'i'));
}

#define GET(r, rn, o) ( (r[rn] & o) >> ofs)
#define GETR(r) GET(r, reg, off)

int main(int argc, char *argv[])
{
	char s[11];
	char *sp;

	printf("Enter GPLR 0-2\n");
	for(int k=0; k<3; k++) {
		sp = s;
		for(int i=0; i<10; i++) {
			*sp = getchar();
			sp++;
		}
		getchar();
		GPLR[k] = strtoul(s, NULL, 16);
	}
	
	printf("Enter GPDR 0-2\n");
	for(int k=0; k<3; k++) {
		sp = s;
		for(int i=0; i<10; i++) {
			*sp = getchar();
			sp++;
		}
		getchar();
		GPDR[k] = strtoul(s, NULL, 16);
	}
	
	printf("Enter GRER 0-2\n");
	for(int k=0; k<3; k++) {
		sp = s;
		for(int i=0; i<10; i++) {
			*sp = getchar();
			sp++;
		}
		getchar();
		GRER[k] = strtoul(s, NULL, 16);
	}
	
	printf("Enter GFER 0-2\n");
	for(int k=0; k<3; k++) {
		sp = s;
		for(int i=0; i<10; i++) {
			*sp = getchar();
			sp++;
		}
		getchar();
		GFER[k] = strtoul(s, NULL, 16);
	}
	
	printf("Enter GEDR 0-2\n");
	for(int k=0; k<3; k++) {
		sp = s;
		for(int i=0; i<10; i++) {
			*sp = getchar();
			sp++;
		}
		getchar();
		GEDR[k] = strtoul(s, NULL, 16);
	}
	
	printf("Enter GAFR 0-2 L/U\n");
	for(int k=0; k<6; k++) {
		sp = s;
		for(int i=0; i<10; i++) {
			*sp = getchar();
			sp++;
		}
		getchar();
		GAFR[k] = strtoul(s, NULL, 16);
	}
	
	printf("\nGPIO Map\n");
	for(int i=0; i<90; i++) {
		int reg = i / 31;
		int ofs = i % 31;
		int off = 1 << ofs;
		
		int reg_a = i / 15;
		int ofs_a = (i % 16) * 2;
		int adr = (GAFR[reg_a] & (3 << ofs_a)) >> ofs_a;
		
		//printf("%d: %d/%d and %d/%d\n", i, reg, ofs, reg_a, ofs_a);
		
		printGPIO(i, GETR(GPLR), GETR(GPDR), GETR(GRER), GETR(GFER), GETR(GEDR), adr);
	}
	
	return 0;
}
