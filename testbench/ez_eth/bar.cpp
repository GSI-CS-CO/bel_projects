#include <ctype.h>
#include <iostream>
#include <queue>

typedef std::queue<int> packet;

std::queue<packet> fifo;
int buffer[50];

void enqueuePacket(int* buf, size_t n) {
	packet tmp;
	for(int i=0;i<n;i++) tmp.push(buf[i]);
	fifo.push(tmp);
}

void readOut() {
	while (!fifo.empty()) 
    {
		while (!fifo.front().empty()) 
    	{ 
    	    std::cout << '\t' << fifo.front().front(); 
    	    fifo.front().pop(); 
    	}
    	std::cout << std::endl;  
		fifo.pop();
    }
}

void readOutOne() {
	if (!fifo.empty()) 
    {
		if (!fifo.front().empty()) 
    	{ 
    	    std::cout << '\t' << fifo.front().front(); 
    	    fifo.front().pop(); 
    	} else {
    		std::cout << std::endl;  
			fifo.pop();
		}
    }
}

int main(int argc, char *argv[])
{
	int* bIn = &buffer[0];
	int i;
	for(i=0;i<3;i++) bIn[i] = i;

	enqueuePacket(bIn, i);	
		

	for(i=0;i<8;i++) bIn[i] = i;

	enqueuePacket(bIn, i);

	//readOut();

	while (true) {
		if(!fifo.empty()) readOutOne();
	}

	return 0;
}	