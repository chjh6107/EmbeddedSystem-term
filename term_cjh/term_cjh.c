#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include "font.h"

#ifndef TEXTLCD_H_
#define TEXTLCD_H_

#define TEXTLCD_ON 				1
#define TEXTLCD_OFF 			2
#define TEXTLCD_INIT 			3
#define TEXTLCD_CLEAR			4
#define TEXTLCD_LINE1			5
#define TEXTLCD_LINE2			6
#define TEXTLCD_CGRAM			7
#define TEXTLCD_HOME			8
#endif /* TEXTLCD_H_ */

void *StartLedThread(int fd_led);
void *StartPiezoThread(int fd_piezo);
void *StartDotThread(int fd_dot);
void *CountDotStageThread(void); //dotMatrix 什砺戚走 展戚袴
///////////////////////
void *ScoreThread(int fd_seg);
void *LedStageThread(int fd_led);
void *PiezoBgmThread(int fd_piezo);
void *MainCountThread(int fd_dot);
void *CountMainDotThread(void);
void *InputAnswerThread(int fd_keypad);
	void *CountKeypadThread(void); //Keypad 朝錘斗

int StageLCD(int fd_text); // 朝錘斗 獣拙 穿 LCD拭 什砺戚走研 析獣旋生稽 窒径
int StageStartAlert(int fd_led, int fd_piezo, int fd_dot);
int StageMain(int fd_piezo,int fd_text, int fd_keypad, int fd_dot, int fd_led);
	int CompaerAnswer(char*,int);

int SuccessAlert(int fd_piezo, int fd_seg);
void *SuccessPiThread(int fd_piezo);
void *SuccessSegThread(int fd_seg);

void *FailPiThread(int fd_piezo);

int fd_text;
int dotstg_cntr; //dotmatrix 什砺戚走 展戚袴研 是廃 痕呪
int stage; //唖 惟績 什砺戚走
int score=0; //惟績 score
int stop = 0; //唖曽 雌伐拭辞 thread研 怪 呪 赤亀系 馬澗 重硲 痕呪
int key_timer=0; //keypad 展戚袴研 是廃 痕呪
int res=0; //惟績 衣引
char question[10]={}; //庚薦
char answer[10]={}; //岩
int dot_main_count = -1;

int main(){
	int fd_led, fd_seg, fd_piezo, fd_dot, fd_keypad;
	pthread_t score_t,failpiezo_t;
	res =0;
	fd_led = open("/dev/fpga_led_cjh",O_WRONLY);
	fd_seg = open("/dev/fpga_segment_cjh",O_WRONLY);
	fd_piezo = open("/dev/fpga_piezo_cjh",O_WRONLY);
	fd_dot = open("/dev/fpga_dotmatrix_cjh",O_WRONLY);
	fd_keypad = open("/dev/fpga_keypad_cjh",O_RDWR);
	fd_text = open("/dev/fpga_textlcd_cjh", O_WRONLY);

	assert(fd_led != -1);
	assert(fd_seg != -1);
	assert(fd_piezo != -1);
	assert(fd_dot != -1);
	assert(fd_keypad != -1);
	assert(fd_text != -1);

	ioctl(fd_text, TEXTLCD_INIT);

	pthread_create(&score_t,NULL,&ScoreThread,fd_seg);//什坪嬢研 窒径馬澗 什傾球 持失
	for(stage = 1 ; stage <= 8 ; stage++){ //恥 8什砺戚走猿走 叔楳
		printf("---stage %d ---\n",stage);
		res = 0, stop = 0;
		 //StageStartAlert敗呪 鎧 thread級戚 舛雌旋生稽 曽戟鞠醸澗走 端滴馬奄 是廃 痕呪
		int alert_checker=0;
		printf("alert_checker = %d\n", alert_checker);

		StageLCD(fd_text);
		//什砺戚走 獣拙 獣 朝錘闘研 馬奄 是廃 敗呪. return葵聖 alert_checker拭 煽舌
		alert_checker = StageStartAlert(fd_led, fd_piezo, fd_dot);
		printf("alert_checker = %d\n",alert_checker);

		//是 敗呪 鎧 thread亜 舛雌旋生稽 曽戟鞠檎 什砺戚走 獣拙
		if(alert_checker == 1){
			StageMain(fd_piezo, fd_text, fd_keypad, fd_dot, fd_led); //Main 惟績 敗呪

			if(res == -1){
				pthread_create(&failpiezo_t,NULL,&FailPiThread,fd_piezo);
				printf("置曽 什坪嬢澗 %d 脊艦陥 \n",score);
				printf("------FAILED------\n\n");
				pthread_join(failpiezo_t,NULL);
				return 0;
			}

			if(res == 1){
				SuccessAlert(fd_piezo, fd_seg);
				printf("薄仙 什坪嬢澗 %d 脊艦陥\n",score);
				printf("-----WIN-----\n");
			}
			if(stage ==8 && res == 1){
				SuccessAlert(fd_piezo, fd_seg);
				printf("置板税 渋切!!\n");
				printf("===================\n");
				printf(" Y O U R S C O R E\n");
				printf("=======%d======\n\n",score);
				return 0;
			}
		}

		printf("\n\n");
	}

	ioctl(fd_text, TEXTLCD_OFF);
	close(fd_led);
	close(fd_piezo);
	close(fd_seg);
	close(fd_dot);
	close(fd_keypad);
	close(fd_text);

	return 0;
}

int StageLCD(int fd_text){
	printf("called StageLCD\n");
	char *outputstg = "    ==STAGE==";
	char *outputnum = malloc(sizeof(char) * 16);
	char *transnum = malloc(sizeof(char) * 4); //舛呪莫 什砺戚走研 char莫生稽 痕発 板 煽舌馬澗 痕呪

	strcpy(outputnum,"        ");
	sprintf(transnum,"%d",stage); //stage研 庚切伸稽 痕発 板 transnum拭 煽舌
	strcat(outputnum,transnum); //因拷 及拭 transnum 細績

	ioctl(fd_text, TEXTLCD_CLEAR);
	ioctl(fd_text, TEXTLCD_LINE1); //textlcd 湛属 匝拭 outputstg 窒径
	write(fd_text, outputstg, strlen(outputstg));
	ioctl(fd_text, TEXTLCD_LINE2); //textlcd 却属 匝拭 outputnum 窒径
	write(fd_text, outputnum, strlen(outputnum));
}

//唖 巨郊戚什稽 朝錘闘 陥錘聖 是廃 敗呪
int StageStartAlert(int fd_led, int fd_piezo, int fd_dot){
	printf("called StageStartAlert\n");
	//惟績 獣拙 採歳聖 窒径馬奄 是廃 什傾球 痕呪
	pthread_t alert_led_t, alert_piezo_t, alert_dot_t;

	pthread_create(&alert_led_t, NULL, &StartLedThread, fd_led);
	pthread_create(&alert_piezo_t, NULL, &StartPiezoThread, fd_piezo);
	pthread_create(&alert_dot_t, NULL, &StartDotThread, fd_dot);

	if(pthread_join(alert_led_t,NULL) == 0
			&& pthread_join(alert_piezo_t,NULL) == 0
			&& pthread_join(alert_dot_t,NULL) == 0){
		printf("StageStartAlert function Job done\n");
		return 1; //thread級戚 舛雌曽戟 馬檎 1聖 return
	}
}

int StageMain(int fd_piezo, int fd_text, int fd_keypad, int fd_dot, int fd_led){
	printf("called StageMain\n");

	int i;
	pthread_t piezo_bgm, main_counter, counter, led_stage_t;


	srand(time(NULL)); //貝呪 降持

	//16遭呪 壕伸
	char hex_arr[16] = {'0','1','2','3','4','5','6','7',
						'8','9','A','B','C','D','E','F'};

	for(i=0;i<10;i++){//古 stage原陥 question 壕伸拭 歯稽錘 hex葵 脊径
		question[i]=hex_arr[rand() % 16];//16遭呪 掻 馬蟹研 question 壕伸拭 脊径
		answer[i]='\0';//什砺戚走 獣拙原陥 answer 段奄鉢
	}
	printf("question = %s\n",question);

	pthread_create(&led_stage_t, NULL, &LedStageThread, fd_led);
	pthread_create(&piezo_bgm, NULL, &PiezoBgmThread, fd_piezo);
	pthread_create(&main_counter,NULL, &MainCountThread, fd_dot);
	pthread_create(&counter,NULL,&CountMainDotThread,NULL); //朝錘斗 獣拙

	ioctl(fd_text,TEXTLCD_CLEAR);
	ioctl(fd_text,TEXTLCD_LINE1);
	write(fd_text,question,strlen(question));

	Compare_answer(question,fd_keypad);


	if(pthread_join(led_stage_t, NULL) == 0
			&&pthread_join(piezo_bgm,NULL) == 0
			&& pthread_join(main_counter,NULL)==0
			&& pthread_join(counter,NULL) ==0)
		printf("StageMain done\n");
}

/*-------------------戚馬 StageStartAlert 敗呪税 THREAD-------------------*/
void *StartLedThread(int fd_led){ //什砺戚走 朝錘闘 LED thread
	int i, led_val=231;

		for(i=3;i>0;i--){ //3段娃 朝錘闘

			write(fd_led,&led_val,1);	//ししし - - ししし

			if(led_val == 231) 			//しし- -  - - しし
				led_val = 195;
			else if(led_val == 195)		//し - - - - - - し
				led_val = 129;

			sleep(1);
		}
		led_val =0; //原走厳 - - - - - - - -
		write(fd_led,&led_val,1);
		sleep(1);
		pthread_exit(NULL);
}

void *StartPiezoThread(int fd_piezo){ //什砺戚走 朝錘闘 紫錘球 thread
	int i;
	int val[]={0x03,0x00,0x03,0x00,0x03,0x00,0x13,0x00};
	int len[]={800000,200000,800000,200000,800000,200000,800000,200000};

	for(i=0; i<sizeof(val);i++){
		write(fd_piezo,&val[i],1);

		if(i==7) //thread亜 襖走走 省澗 神嫌亜 降持馬食 鋼差庚 原走厳析 凶 曽戟
			pthread_exit(NULL);

		usleep(len[i]);
	}
}

void *StartDotThread(int fd_dot){ //什砺戚走 朝錘闘 dotmatrix thread
	int i, j;
	char result[20], tmp[2];
	pthread_t counter;
	pthread_create(&counter,NULL,&CountDotStageThread,NULL); //朝錘斗 獣拙

	for(;;){
		for(i=0;i<2;i++){
			int offset =0;
			if((i % 2) != 0)
				offset=10;

			for (j = 0; j < 5; j++) {
				if(dotstg_cntr == 0)		//03
					sprintf(tmp, "%x%x", font_three[i][j] / 16, font_three[i][j] % 16);
				else if(dotstg_cntr == 1)	//02
					sprintf(tmp, "%x%x", font_two[i][j] / 16, font_two[i][j] % 16);
				else if(dotstg_cntr == 2)	//01
					sprintf(tmp, "%x%x", font_one[i][j] / 16, font_one[i][j] % 16);
				else if(dotstg_cntr == 3)	//獣拙
					sprintf(tmp, "%x%x", font_sizak[i][j] / 16, font_sizak[i][j] % 16);
				else if(dotstg_cntr == 4)	//dotmatrix clear
					sprintf(tmp, "%x%x", font_clr[i][j] / 16, font_clr[i][j] % 16);

				result[offset++] = tmp[0];
				result[offset++] = tmp[1];
			}
			usleep(2000);
		write(fd_dot,&result[0],20);
		}
		if(dotstg_cntr == 4)
			pthread_exit(NULL);
	}
}

//dotmatrix 接雌反引研 紫遂馬奄 是背 紫遂馬澗 展戚袴thread
void *CountDotStageThread(void){
	int i;
	dotstg_cntr = 0;

	for(i = 0; i < 3 ; i ++){
		sleep(1);
		dotstg_cntr++; //原走厳拭澗 3猿走 刊旋 板 鋼差庚 匙閃蟹身
	}
	usleep(500000); //獣拙採歳精 0.5段幻 窒径
	dotstg_cntr = 4; //dotmatrix clear

	pthread_exit(NULL);
}

/*------------------------StageStartAlert 敗呪 Thread 魁----------*/

/*------------------------StageMain 敗呪税 Thread------------------*/

void *LedStageThread(int fd_led){
	int val;

	if(stage == 1)
		val = 128;
	else if(stage == 2)
		val = 192;
	else if(stage == 3)
		val = 224;
	else if(stage == 4)
		val = 240;
	else if(stage == 5)
		val = 248;
	else if(stage == 6)
		val = 252;
	else if(stage == 7)
		val = 254;
	else if(stage == 8)
		val = 255;

	write(fd_led,&val,1);

	if(stop==1)
		pthread_exit(NULL);
}
//惟績税 bgm 窒径

void *PiezoBgmThread(int fd_piezo){
	int i;

	int val[] = { 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x07, 0x00, 0x06, 0x00,
			0x05, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x00,
			0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x12, 0x00, 0x06, 0x00, //湛社箭
			0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x07, 0x00, 0x06, 0x00,
			0x05, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x00,
			0x11, 0x00, 0x11, 0x00, 0x07, 0x00, 0x07, 0x00,//砧腰属
			0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x07, 0x00, 0x06, 0x00,
			0x05, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x00,
			0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x12, 0x00, 0x06, 0x00, //湛社箭
			0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x07, 0x00, 0x06, 0x00,
			0x05, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x00,
			0x11, 0x00, 0x11, 0x00, 0x07, 0x00, 0x07, 0x00,
			0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x07, 0x00, 0x06, 0x00,
			0x05, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x00,
			0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x12, 0x00, 0x06, 0x00, //湛社箭
			0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x05, 0x00,
			0x04, 0x00, 0x03, 0x00, 0x07, 0x00, 0x06, 0x00,
			0x05, 0x00, 0x06, 0x00, 0x05, 0x00, 0x04, 0x00,
			0x11, 0x00, 0x11, 0x00, 0x07, 0x00, 0x07, 0x00};

	int len[] = {150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 500000, 1000, 500000, 1000, //湛社箭
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 100000,
			150000, 3000, 500000, 150000, 150000, 3000, 500000, 50000,//砧腰属
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 500000, 1000, 500000, 1000, //湛社箭
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 100000,
			150000, 3000, 500000, 150000, 150000, 3000, 500000, 50000,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 500000, 1000, 500000, 1000, //湛社箭
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 1500,
			150000, 1500, 150000, 1500, 150000, 1500, 150000, 100000,
			150000, 3000, 500000, 150000, 150000, 3000, 500000, 50000};

	for(i=0; i<sizeof(val);i++){
		write(fd_piezo,&val[i],1);

		if(stop == 1)
			pthread_exit(NULL);
		if(i==215)
			pthread_exit(NULL);
		usleep(len[i]);
	}

}

void *MainCountThread(int fd_dot){

	int i, j;
	char result[20], tmp[2];

	for(;;){
		for(i=0;i<2;i++){
			int offset =0;
			if((i % 2) != 0)
				offset=10;

			for (j = 0; j < 5; j++) {
				if(dot_main_count == 20)		//20
					sprintf(tmp, "%x%x", font_20[i][j] / 16, font_20[i][j] % 16);
				else if(dot_main_count == 19)	//19
					sprintf(tmp, "%x%x", font_19[i][j] / 16, font_19[i][j] % 16);
				else if(dot_main_count == 18)	//08
					sprintf(tmp, "%x%x", font_18[i][j] / 16, font_18[i][j] % 16);
				else if(dot_main_count == 17)	//獣拙
					sprintf(tmp, "%x%x", font_17[i][j] / 16, font_17[i][j] % 16);
				else if(dot_main_count == 16)
					sprintf(tmp, "%x%x", font_16[i][j] / 16, font_16[i][j] % 16);
				else if(dot_main_count == 15)
					sprintf(tmp, "%x%x", font_15[i][j] / 16, font_15[i][j] % 16);
				else if(dot_main_count == 14)
					sprintf(tmp, "%x%x", font_14[i][j] / 16, font_14[i][j] % 16);
				else if(dot_main_count == 13)
					sprintf(tmp, "%x%x", font_13[i][j] / 16, font_13[i][j] % 16);
				else if(dot_main_count == 12)
					sprintf(tmp, "%x%x", font_12[i][j] / 16, font_12[i][j] % 16);
				else if(dot_main_count == 11)
					sprintf(tmp, "%x%x", font_11[i][j] / 16, font_11[i][j] % 16);
				else if(dot_main_count == 10)
					sprintf(tmp, "%x%x", font_ten[i][j] / 16, font_ten[i][j] % 16);
				else if(dot_main_count == 9)
					sprintf(tmp, "%x%x", font_nine[i][j] / 16, font_nine[i][j] % 16);
				else if(dot_main_count == 8)
					sprintf(tmp, "%x%x", font_eight[i][j] / 16, font_eight[i][j] % 16);
				else if(dot_main_count == 7)
					sprintf(tmp, "%x%x", font_seven[i][j] / 16, font_seven[i][j] % 16);
				else if(dot_main_count == 6)
					sprintf(tmp, "%x%x", font_six[i][j] / 16, font_six[i][j] % 16);
				else if(dot_main_count == 5)
					sprintf(tmp, "%x%x", font_five[i][j] / 16, font_five[i][j] % 16);
				else if(dot_main_count == 4)
					sprintf(tmp, "%x%x", font_four[i][j] / 16, font_four[i][j] % 16);
				else if(dot_main_count == 3)
					sprintf(tmp, "%x%x", font_three[i][j] / 16, font_three[i][j] % 16);
				else if(dot_main_count == 2)
					sprintf(tmp, "%x%x", font_two[i][j] / 16, font_two[i][j] % 16);
				else if(dot_main_count == 1)
					sprintf(tmp, "%x%x", font_one[i][j] / 16, font_one[i][j] % 16);

				result[offset++] = tmp[0];
				result[offset++] = tmp[1];
			}
			usleep(2000);
		write(fd_dot,&result[0],20);
		}

		if(stop == 1)
			pthread_exit(NULL);


	}
}

void *CountMainDotThread(void){
	int i;

	dot_main_count = 22 - stage*2;

	for(i = dot_main_count; i > 0 ; i--){
		sleep(1);
		dot_main_count -= 1;
		if(stop == 1){
			pthread_exit(NULL);
		}
	}
	if(i == 0){
		res = -1;
		stop = 1;
		pthread_exit(NULL);
	}
}

int Compare_answer(char* question, int fd_keypad){
	int i;
	printf("called Compare_answer\n");

	pthread_t input_t;

	printf("question on func = %s\n",question);
	pthread_create(&input_t, NULL, &InputAnswerThread, fd_keypad);

	if(pthread_join(input_t, NULL)==0)
		printf("Compare_answer function Job done\n");
}

 //7-segment拭 什坪嬢研 窒径馬澗 thread
void *ScoreThread(int fd_seg){
	char nums[7];

		for(;;){
			sprintf(nums,"%06d",score);
			write(fd_seg, nums, 6);
		}
}

//岩照聖 脊径馬澗 thread
void *InputAnswerThread(int fd_keypad){
	int i,j, prev_t=0, check_t;
	pthread_t keypad_counter;
	char buf[10]={}, check_buf[10]={}, prev_buf[10] = {};

	pthread_create(&keypad_counter,NULL,&CountKeypadThread,NULL); //展戚袴 獣拙

	for(;;){
			read(fd_keypad, buf, 1);
			for(i=0;i<sizeof(buf);i++)
				check_buf[i] = buf[i];
			check_t = key_timer;

			if((buf[0] != prev_buf[0]) || (prev_t != key_timer)){
				for(i=0;i<10;i++){
					if(answer[i] == '\0' ){
						answer[i] = buf[0];

						if(question[i] == answer[i])
							score += 100;

						if(answer[i] != NULL && (question[i] != answer[i])){
							stop =1;
							res=-1; //鳶
							pthread_exit(NULL);
						}
						if(question[9] == answer[9]){
							res = 1; //渋
							stop = 1;
						}
						break;
					}
				}
			}
			if(stop==1)
				pthread_exit(NULL);

			ioctl(fd_text,TEXTLCD_LINE2);
			write(fd_text,answer,strlen(answer));

			for(i=0;i<sizeof(buf);i++)
				prev_buf[i]=buf[i];
			prev_t = key_timer;
			usleep(5000);
			memset(buf,0,sizeof(buf));
			j++;
	}
	if(pthread_join(keypad_counter,NULL) == 0)
		printf("CountKeypad done\n");


}
//keypad 脊径聖 据拝備 馬奄 是廃 thread 敗呪
void *CountKeypadThread(void){
	int i;
	key_timer =0;

	for(i=0;i<10;i++){
	//	printf("Count keypad time = %d\n",key_timer);
		key_timer++;

		if(stop == 1)
			pthread_exit(NULL);
		usleep(50000);
	}
		pthread_exit(NULL);
}
/*------------------------StageMain 敗呪税 Thread------------------*/

int SuccessAlert(int fd_piezo, int fd_seg){
	pthread_t success_piezo_t,success_dot_t;

	pthread_create(&success_piezo_t,NULL,&SuccessPiThread,fd_piezo);
	if(stage == 8){
		pthread_create(&success_dot_t, NULL, &SuccessSegThread, fd_seg);
	}
	pthread_join(success_piezo_t,NULL);
}

void *SuccessPiThread(int fd_piezo){
	int i;
		int val[]={0x01,0x00,0x11,0x00,0x21,0x00};
		int len[]={300000,1000,300000,1000,300000,1000};

		for(i=0; i<sizeof(val);i++){
			write(fd_piezo,&val[i],1);

			if(i==6) //thread亜 襖走走 省澗 神嫌亜 降持馬食 鋼差庚 原走厳析 凶 曽戟
				pthread_exit(NULL);

			usleep(len[i]);
		}
}

void *SuccessSegThread(int fd_seg){
	char nums[7];
	int i;
		for(i=0;i<1000;i++){
			sprintf(nums,"%06d",score);
			write(fd_seg, nums, 6);
		}
}

void *FailPiThread(int fd_piezo){
	int i;
		int val[]={0x06,0x00,0x06,0x00,0x05,0x00,0x06,0x00,0x06,0x00,0x05,0x00};
		int len[]={300000,10000,300000,10000,400000,1000,300000,10000,300000,10000,700000,1000};

		for(i=0; i<sizeof(val);i++){
			write(fd_piezo,&val[i],1);

			if(i==12) //thread亜 襖走走 省澗 神嫌亜 降持馬食 鋼差庚 原走厳析 凶 曽戟
				pthread_exit(NULL);

			usleep(len[i]);
		}
}
