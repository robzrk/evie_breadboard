#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pigpio.h>

//globals
const int display_usec = 20000;
const float period = 1;

#define SRCLK_GPIO 17
#define SER_GPIO 27
#define OE_GPIO 22
#define RCLK_GPIO 23
#define SRCLR_GPIO 24

//pi = pigpio.pi()

void gpio_init(int gpio_pin) {
  gpioSetMode(gpio_pin, PI_OUTPUT);
  gpioSetPullUpDown(gpio_pin, PI_PUD_UP);
  //setwarnings(False);
  //setup(gpio_pin,GPIO.OUT);
}

int gpio_setup(void) {
  if (gpioInitialise() < 0) {
    printf("Failed to initialize PiGPIO library!");
    return -1;
  }
  gpio_init(SRCLK_GPIO);
  gpio_init(SER_GPIO);
  gpio_init(OE_GPIO);
  gpio_init(RCLK_GPIO);
  gpio_init(SRCLR_GPIO);

  return 0;
}

const char* led_to_str(int gpio_pin) {
  switch (gpio_pin) {
  case OE_GPIO: return "OE";
  case SRCLR_GPIO: return "SRCLR";
  case SRCLK_GPIO: return "SRCLK";
  case RCLK_GPIO: return "RCLK";
  case SER_GPIO: return "SER";
  }
  return "UNKNOWN";
}

void led_on(int gpio_pin) {
  //printf("%s: on\n", led_to_str(gpio_pin));
  gpioWrite(gpio_pin,PI_ON);
}

void led_off(int gpio_pin) {
  //printf("%s: off\n", led_to_str(gpio_pin));
  gpioWrite(gpio_pin,PI_OFF);
}


void clear(void) {
  led_off(OE_GPIO); //output enabled
  led_off(SRCLR_GPIO);
  led_off(SRCLK_GPIO);
  led_off(SER_GPIO);
  led_off(RCLK_GPIO);
  gpioDelay(period);

  led_on(SRCLK_GPIO);
  gpioDelay(period);

  led_off(SRCLK_GPIO);
  led_on(SRCLR_GPIO);
  gpioDelay(period);
}

void display_bitmap(unsigned int bitmap) {
  //printf("bitmap: 0x%06x\n", bitmap);
  led_on(OE_GPIO); //output disabled	
  for (int j=0; j<25; ++j) {
    if (bitmap & (1 << j)) {
      led_on(SER_GPIO);
    } else {
      led_off(SER_GPIO);
    }
    
    led_on(RCLK_GPIO);
    led_off(SRCLK_GPIO);
    gpioDelay(period);
    led_off(RCLK_GPIO);
    led_on(SRCLK_GPIO);
    gpioDelay(period);
  }
  led_off(OE_GPIO); //output enabled
  gpioDelay(display_usec);
}

void simple_chaser(void) {
  unsigned int bitmap_list[24];
  unsigned int bitmap;
  for (int i=0; i<24; ++i) {
    bitmap_list[i] = 1<<i;
  }
  while (1) {
    for (int i=0; i<24; ++i) {
      clear();
      bitmap = bitmap_list[i];
      display_bitmap(bitmap);
    }
  }
}

void chase_back() {
  int last_fg_color = 1;
  int fg_color = 2;
  int last_bg_color = 3;
  int bg_color = 4;
  int every_other = 0;
  unsigned int bitmap_list[24];
  unsigned int fg_bitmap;
  unsigned int bg_bitmap;
  int k;
  int max_fg_itr = 4;
  int max_bg_itr = max_fg_itr * 8;
  int bg_itr = 0;
  int fg_itr = 0;
  unsigned int mask;
  int bail_cnt = 0;
  while (1) {
    fg_bitmap = 0;
    bg_bitmap = 0;
      
    if (bg_itr++ >= max_bg_itr) {
      while (bg_color == last_bg_color) {
	bg_color = rand()%8;
      }
      bg_itr = 0;
    }
    if (fg_itr++ >= max_fg_itr) {
      bail_cnt = 0;
      while ((fg_color == last_fg_color || (fg_color & bg_color) == fg_color)
	     && (bail_cnt++ < 1000)) {
	fg_color = rand()%8;
      }
      fg_itr = 0;
    }
    if (fg_color & 0x1) // set R component
      fg_bitmap |= 0x10000;
    if (fg_color & 0x2) // set G component
      fg_bitmap |= 0x100;
    if (fg_color & 0x4) // set B component
      fg_bitmap |= 0x1;

    if (bg_color & 0x1) // set R component
      bg_bitmap |= 0xff0000;
    if (bg_color & 0x2) // set G component
      bg_bitmap |= 0xff00;
    if (bg_color & 0x4) // set B component
      bg_bitmap |= 0xff;
    
    for (int i=0; i<8; ++i) {
      if (every_other)
	k = 7-i;
      else
	k = i;
      // 111110
      // 111101
      // 111011
      // 110111
      // 101111
      // 011111
      switch (i) {
      case 0:
	mask = 0xfefefe;
	break;
      case 1:
	mask = 0xfdfdfd;
	break;
      case 2:
	mask = 0xfbfbfb;
	break;
      case 3:
	mask = 0xf7f7f7;
	break;
      case 4:
	mask = 0xefefef;
	break;
      case 5:
	mask = 0xdfdfdf;
	break;
      case 6:
	mask = 0xbfbfbf;
	break;
      case 7:
      default:
	mask = 0x7f7f7f;
	break;
      }
      
      bitmap_list[k] = (bg_bitmap & mask) | fg_bitmap<<i;
      //bitmap_list[k] = bg_bitmap | (fg_bitmap<<i);
    }
    //printf("fg: 0x%x bg: 0x%x\n", fg_color, bg_color);
    //printf("fg_bitmap: 0x%06x, bg_bitmap: 0x%06x\n", fg_bitmap, bg_bitmap);
    for (int i=0; i<8; ++i) {
      display_bitmap(bitmap_list[i]);
      clear();
    }
    if (every_other == 0)
      every_other = 1;
    else
      every_other = 0;
    last_fg_color = fg_color;
    last_bg_color = bg_color;
  }
}

void test_fn(void) {
  gpioInitialise();
  gpioSetMode(22, PI_OUTPUT);
  gpioWrite(22,1);
  gpioDelay(5);
  gpioTerminate();
}

// Need to run as sudo
// Need to kill pgpiod first
int main(void) {
  printf("Dispay Usec: %d, Period: %f\n", display_usec, period);
  gpio_setup();
  //simple_chaser();
  chase_back();
  //test_fn();
  gpioTerminate();
  return 0;
}
 
    
