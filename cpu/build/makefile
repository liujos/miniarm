EXEC:=cpu
SDIR:=../src
IDIR:=../inc
CFLAGS:=-I $(IDIR)
OBJS:=cpu.o execute.o mem_op.o utils.o

$(EXEC):$(OBJS) $(DEPS)
	$(CC) -o $@ $(OBJS) $(CFLAGS)

$(OBJS):%.o:$(SDIR)/%.c
	$(CC) -c -o $@ $^ $(CFLAGS)

clean:
	rm $(EXEC) *.o

