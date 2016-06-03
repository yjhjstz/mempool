
EXEC = test  
OBJS = $(SRC:%.c=%.o)  
SRC  = $(wildcard *.c)

CC = gcc  
CFLAGS += -g -Wall
LDFLAGS +=   

all:$(EXEC)  

$(EXEC):$(OBJS)  
	$(CC) $(LDFLAGS) -o $@ $(OBJS)  

%.o:%.c  
	$(CC) $(CFLAGS) -c $< -o $@  

clean:  
	@rm -vf $(EXEC) *.o *~  
