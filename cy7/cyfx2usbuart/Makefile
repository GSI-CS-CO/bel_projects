CC = sdcc -mmcs51
BASENAME = convert_serial
CYCFX2PROG = cycfx2prog

$(BASENAME).ihx: $(BASENAME).c
	$(CC) -c softuart.c
	$(CC) $(BASENAME).c softuart.rel

clean:
	rm -f $(BASENAME).{asm,ihx,lk,lnk,lst,map,mem,rel,rst,sym}
	rm -f softuart.{asm,ihx,lk,lnk,lst,map,mem,rel,rst,sym}

.PHONY:run
run: $(BASENAME).ihx
	$(CYCFX2PROG) prg:$(BASENAME).ihx run
