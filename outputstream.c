#include "outputstream.h"

char mydata[45];

PI_THREAD (OutputStream)
{
	// printf("OutputStream thread started\n");
	while(1)
	{
		// printf("OutputStream thread continues\n");

		// Loop if we have a buffer under-run
		while (bufferGet(streamBuffer,mydata))
		{
			delay(10);
		}

		fwrite(&mydata[3],1,42,stdout);
			// printf("%c",mydata[j]);
		//	printf("[FillFIFO] GOT STUFF !!!!!!!! bufferGet \n");
	}
}
