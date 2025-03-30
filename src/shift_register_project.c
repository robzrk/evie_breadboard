#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>

//globals
const int display_usec = 50000;
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

  led_on(SRCLK_GPIO);
  gpioDelay(period);
}

void simple_chaser(void) {
  unsigned int bitmap_list[25];
  unsigned int bitmap;
  for (int i=0; i<24; ++i) {
    bitmap_list[i] = 1<<i;
  }
  while (1) {
    for (int i=0; i<24; ++i) {
      clear();
      bitmap = bitmap_list[i];
      //printf("bitmap: 0x%x\n", bitmap);
      led_on(OE_GPIO); //output disabled	
      for (int j=0; j<25; ++j) {
	if (bitmap & (1 << j)) {
	  led_on(SER_GPIO);
	} else {
	  led_off(SER_GPIO);
	}
	led_off(RCLK_GPIO);
	led_off(SRCLK_GPIO);
	gpioDelay(period);
	led_on(RCLK_GPIO);
	led_on(SRCLK_GPIO);
	gpioDelay(period);
      }
      led_off(OE_GPIO); //output enabled
      gpioDelay(display_usec);
    }
  }
}

/* def chase_back(): */
/*     last_color = 1 */
/*     color = 2 */
/*     every_other = 0 */
/*     while 1: */
/*         bitmap_list = [] */
/*         bitmap = 0 */
/*         while color == last_color or color == 0: */
/*             color = random.randint(0,7) */
/*         if (color & 0x1): */
/*             bitmap |= 0x10000 */
/*         if (color & 0x2): */
/*             bitmap |= 0x100 */
/*         if (color & 0x4): */
/*             bitmap |= 0x1 */
/*         for i in range(8): */
/*             bitmap_list += [bitmap<<i] */
/*         for bitmap in bitmap_list: */
/*             clear() */
/*             led_on(OE_GPIO) #output disabled */
/*             for j in range(24): */
/*                 if every_other == 0: */
/*                     k = j */
/*                 else: */
/*                     k = 23 - j */
/*                 if (bitmap & (1 << k)): */
/*                     led_on(SER_GPIO) */
/*                 else: */
/*                     led_off(SER_GPIO) */
/*                 led_off(RCLK_GPIO) */
/*                 led_on(SRCLK_GPIO) */
/*                 gpioDelay(period) */
/*                 led_on(RCLK_GPIO) */
/*                 led_off(SRCLK_GPIO) */
/*                 gpioDelay(period) */
/*             led_off(OE_GPIO) #output enabled */
/*             gpioDelay(display_usec) */
/*         if every_other == 0: */
/*             every_other = 1 */
/*         else: */
/*             every_other = 0 */
/*         last_color = color */


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
  simple_chaser();
  //chase_back();
  //test_fn();
  gpioTerminate();
  return 0;
}
 
    
