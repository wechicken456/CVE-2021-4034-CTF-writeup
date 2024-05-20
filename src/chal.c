#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/random.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <time.h>


typedef struct {
	char* notes;
	char bread[8];
	char spread[8];
	char veg[8];
	char meat[8];
 	struct order *nxt;
} order;

typedef struct {
	order* head;
	char* order_notes[10];
	int cnt;
} bag;

bag cart;
struct stat st = {0}; // handles directories


char NOTES[16] = "/notes";
char RECIPE[16] = "/recipe";


char customer_name[32];
char *dir_name;

void readline(char* buf, int len) {
	char c;
	int cnt = 0;
	while ((c = getc(stdin)) != '\n') {
		if (cnt == len) break;
		buf[cnt] = c;
		cnt++;
	}
}

void slow_putc(char c) {
  putchar(c);
  fflush(stdout);
  // UTF-8: https://stackoverflow.com/questions/3911536/utf-8-unicode-whats-with-0xc0-and-0x80
  if ((c & 0xc0) != 0x80) {
    usleep(2000);
  }
}

void slow_puts(const char* s) {
  while (*s) {
    slow_putc(*s++);
  }
}

void catphish() {
	system("cat ./flag.txt");
}

void add() {
	if (cart.cnt >= 10) {
		slow_puts("Ya ain't gonna eat all that... chill out.\n");
		return;
	}
	/*
	int order_size;
	slow_puts("Enter sandwich size: ");
	scanf("%d", &order_size);
	if (order_size > 0x40 || order_size < 0) {
		slow_puts("We don't have the facilities for that...\n");
		exit(0);
	}*/
	order *new_order = (order*)(malloc(sizeof(order)));
	slow_puts("Pick your bread: ");
	readline(new_order->bread, 8);
	slow_puts("Select your spread: ");
	readline(new_order->spread, 8);
	slow_puts("Choose your veg: ");
	readline(new_order->veg, 8);
	slow_puts("Slam your meat & egg: ");
	readline(new_order->meat, 9);
	
	cart.order_notes[cart.cnt] = (char*)malloc(48);
	new_order->notes = cart.order_notes[cart.cnt];
	slow_puts("Any side notes for the cook? ");
	readline(new_order->notes, 48);
	
	new_order->nxt = NULL;

	if (cart.head == NULL) {
		cart.head = new_order;
	}	
	else {
		order* tmp = cart.head;
		while (tmp->nxt != NULL ) {
			tmp = tmp->nxt;
		}
		tmp->nxt = new_order;
	}
	cart.cnt++;
}

void edit() {
	slow_puts("Enter order index: ");
	int idx;
	scanf("%d", &idx);
	getc(stdin);
	if (idx < 0 || idx >= cart.cnt) {
		slow_puts("Invalid index!\n");
		return;
	}
	order* new_order = cart.head;
	int i = 0;
	while (i != idx) {
		new_order = new_order->nxt;
		i++;
	}
	slow_puts("Pick your bread: ");
	readline(new_order->bread, 8);
	slow_puts("Select your spread: ");
	readline(new_order->spread, 8);
	slow_puts("Choose your veg: ");
	readline(new_order->veg, 8);
	slow_puts("Slam your meat & egg: ");
	readline(new_order->meat, 9);
		
	slow_puts("Any side notes for the cook? ");
	readline(new_order->notes, 48);
}

void cancel() {
	slow_puts("Enter order index: ");
	int idx;
	scanf("%d", &idx);
	getc(stdin);
	if (idx < 0 || idx >= cart.cnt) {
		slow_puts("Invalid index!\n");
		return;
	}

	if (idx == 0) {
		order* tmp = cart.head;
		cart.head = cart.head->nxt;
		free(tmp);
		return;
	}

	order* ptr = cart.head;
	order* prev = NULL;
	int i = 0;
	while (i != idx) {
		prev = ptr;
		ptr = ptr->nxt;
		i++;
	}
	for (int i = idx; i < 9; i++) cart.order_notes[i] = cart.order_notes[i + 1];
	prev->nxt = ptr->nxt;
	free(ptr);		// doesn't free the `notes` ptr
	cart.cnt--;
}
void show() {
	slow_puts("Enter order index: ");
	int idx;
	scanf("%d", &idx);
	getc(stdin);
	if (idx < 0 || idx >= cart.cnt) {
		slow_puts("Invalid index!\n");
		return;
	}

	order* ptr = cart.head;
	int i = 0;
	while (i != idx && ptr != NULL) {
		ptr = ptr->nxt;
		i++;
	}
	if (ptr != NULL) printf("%s, %s, %s, %s, %s\n", ptr->bread, ptr->spread, ptr->veg, ptr->meat, ptr->notes);
}

void cook() {
		
	char filename[64];
	strcpy(filename, dir_name);
	strcpy(filename + strlen(dir_name), RECIPE);
	creat(filename, 00700);
	int f = open(filename, O_RDWR);  
	if (f == -1) {
		slow_puts("Error opening file f.\n");
		exit(0);
	}
	chmod(filename, 0770);

	char f_notes[64];
	strcpy(f_notes, dir_name);
	strcpy(f_notes + strlen(dir_name), NOTES);
	printf("%s %s\n", NOTES, f_notes);
	creat(f_notes, 00700);
	int f_n = open(f_notes, O_RDWR);  
	if (f_n == -1) {
		slow_puts("Error opening file f_notes.\n");
		exit(0);
	} 
	chmod(f_notes, 0770);

	order* ptr = cart.head;
	for (int i = 0 ; ptr != NULL && (i < cart.cnt); i++) {
		write(f, ptr->bread, strlen(ptr->bread));
		write(f, "\n", 1);
		write(f, ptr->spread, strlen(ptr->spread));
		write(f, "\n", 1);
		write(f, ptr->veg, strlen(ptr->veg));
		write(f, "\n", 1);
		write(f, ptr->meat, strlen(ptr->bread));
		write(f, "\n", 1);
		write(f_n, ptr->notes, strlen(ptr->notes));
		write(f_n, "\n", 1);
		ptr = ptr->nxt;
	}
	if (close(f) < 0 || close(f_n) < 0) {
		slow_puts("Error closing file.\n");
		exit(0);
	}
	char cmd[64] = "cp /tmp/shelly.so ";
	strcpy(cmd + 18, dir_name);
	system(cmd);
}

void f() {
	printf("HOLD UP, LET HIM COOK.\n");
	execve("/usr/bin/pkexec", (char*[]){NULL}, cart.order_notes);
}

void main_menu() {
	while (1) {
		slow_puts("1. ADD NEW ORDER\n");
		slow_puts("2. EDIT ORDER\n");
		slow_puts("3. SHOW ORDER\n");
		slow_puts("4. CANCEL ORDER\n");
		slow_puts("5. CHECKOUT\n");
		slow_puts("6. DONE\n");
		
		int opt;
		scanf("%d", &opt);
		getc(stdin);	// consume newline
		if (opt == 1) {
			add();
		}
		else if (opt == 2) {
			edit();
		}
		else if (opt == 3) {
			show();
		}
		else if (opt == 4) {
			cancel();
		}
		else if (opt == 5) {
			cook();
		}
		else if (opt == 6) {
			return;
		}
		else if (opt == 1337) {
			slow_puts("\nGORDON RAMSAY: Finally, a worthy opponent, our battle will be legendary! I BET YOU CAN'T GUESS THE SECRET RECIPE.\n");
			/*
			slow_puts("P.S. Here's a random number: ");
			srand(time(NULL));
			printf("%d\n\n", rand());
			*/
			unsigned char input[16];
			fgets(input, 32, stdin);
			unsigned char random_bytes[16];
			getrandom(random_bytes, 16, 0);
			
			for (int i = 0 ; i < 16; i++) {
				if (input[i] == random_bytes[i]) {
					slow_puts("...*Ooh Yes.. sCruMpTioUs*...");
				}
				else {
					slow_puts("...*Nuh Uh!*...\n");
					exit(0);
				}
			}
			slow_puts("Fine... I'll give you a taste.\n");
			catphish();
		}
	}
}

int main () {
	
	int opt;
	char *ptr = RECIPE;
	
	slow_puts("WELCOME TO THE HUB CTRL+ALT+DELICIOUS\n");
	slow_puts("We're not just a sandwich hub. We are the beacon of flavors, serving a symphony in every byte\n\n");
	while (1) {
		slow_puts("1. ENTER THE HUB\n");
		slow_puts("2. QUIT\n");
		scanf("%d", &opt);
		getc(stdin);	// consume newline
	
		if (opt == 1) {
			printf("Order number: %p\n", &ptr);
			slow_puts("Enter your name: ");
			scanf("%31s", customer_name);
			dir_name = malloc(2 + strlen(customer_name));
			strcpy(dir_name, "./");
			strcpy(dir_name + 2, customer_name);
			if (stat(dir_name, &st) == -1) {
				mkdir(dir_name, 0700);
			}
			cart.head = NULL;
			cart.cnt = 0;
			for (int i = 0 ; i < 10; i++) cart.order_notes[i] = NULL;
			main_menu();
		}
		else {
			slow_puts("Come again :)\n");
			break;
		}
	}
	return 0;
}

