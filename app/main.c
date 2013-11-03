/*
 * main.c
 *
 *  Created on: 2013-10-30
 *      Author: Armink
 */
#include "cache.h"
#include <stdio.h>
int main()
{
	Cache cache;
	uint16_t valueTemp[CACHE_LENGTH_MAX];
	initCache(&cache);
	valueTemp[0] = 0;
	valueTemp[1] = 1;
	valueTemp[2] = 2;
	valueTemp[3] = 3;
	addData(&cache,"温度",1,1,valueTemp,NULL);
	addData(&cache,"压力",2,2,valueTemp,NULL);
	addData(&cache,"湿度",3,3,valueTemp,NULL);
	addData(&cache,"PM2.5",4,4,valueTemp,NULL);
	getValue(&cache,"温度",valueTemp);
	getValue(&cache,"压力",valueTemp);
	getValue(&cache,"湿度",valueTemp);
	getValue(&cache,"PM2.5",valueTemp);
	removeData(&cache,"温度");
	removeData(&cache,"压力");
	removeData(&cache,"湿度");
	removeData(&cache,"PM2.5");
	removeData(&cache,"PM2.5");
	getValue(&cache,"PM2.5",valueTemp);
	addData(&cache,"PM2.5",4,4,valueTemp,NULL);
	getValue(&cache,"PM2.5",valueTemp);
	valueTemp[0] = 3;
	valueTemp[1] = 2;
	valueTemp[2] = 1;
	valueTemp[3] = 0;
	putValue(&cache,"PM2.5",valueTemp);
	getValue(&cache,"PM2.5",valueTemp);
	removeData(&cache,"PM2.5");
	getValue(&cache,"PM2.5",valueTemp);
//	putValue(&cache,"PM2.5",valueTemp);
//	getValue(&cache,"温度",valueTemp);
//	getValue(&cache,"压力",valueTemp);
//	getValue(&cache,"湿度",valueTemp);
//	getValue(&cache,"PM2.5",valueTemp);
//
//	removeData(&cache,"PM2.5");
//	getValue(&cache,"PM2.5",valueTemp);
//	removeData(&cache,"温度");
//	findData(&cache,"温度");
//	addData(&cache,"温度",1,1,valueTemp,NULL);
//	getValue(&cache,"温度",valueTemp);

	return 0;
}
