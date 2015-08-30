#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/fb.h>

#define LCD_X_RES 100
#define LCD_Y_RES 101
#define LCD_BBP 16
#define NBR_PALETTE 256
#define ACTIVE_VIDEO_MEM_OFFSET (LCD_X_RES*START_LINES*(LCD_BBP/8))
#define ACTIVE_VIDEO_MEM_SIZE   (LCD_Y_RES*LCD_X_RES*(LCD_BBP/8))

static int bgr;
module_param(bgr, int, 0);
MODULE_PARM_DESC(bgr, "use bgr?");


static struct fb_info fb_info;
static struct fb_var_screeninfo fb_info_defined = {
	.bits_per_pixel		= LCD_BBP,
	.activate		= FB_ACTIVATE_TEST,
	.xres			= LCD_X_RES,
	.yres			= LCD_Y_RES,
	.xres_virtual		= LCD_X_RES,
	.yres_virtual		= LCD_Y_RES,
	.height			= -1,
	.width			= -1,
	.left_margin		= 0,
	.right_margin		= 0,
	.upper_margin		= 0,
	.lower_margin		= 0,
	.red			= {11, 5, 0},
	.green			= {5, 6, 0},
	.blue			= {0, 5, 0},
	.transp			= {0, 0, 0},
};

static struct fb_fix_screeninfo fb_fix = {
        .id             = KBUILD_MODNAME,
        .smem_len       = ACTIVE_VIDEO_MEM_SIZE,
        .type           = FB_TYPE_PACKED_PIXELS,
        .visual         = FB_VISUAL_TRUECOLOR,
        .xpanstep       = 0,
        .ypanstep       = 0,
        .line_length    = LCD_X_RES*(LCD_BBP/8),
        .accel          = FB_ACCEL_NONE,
};

struct timer_list timeout;

static void timer_handler(unsigned long data)
{
	char *fb_buffer = (char *)data;
	int i = 0;
	int sum = 0;
	for (i = 0; i < ACTIVE_VIDEO_MEM_SIZE; i++) {
		sum += fb_buffer[i];
	}
	printk(KERN_INFO "sum=%d\n", sum);
	mod_timer(&timeout, jiffies + msecs_to_jiffies(500));
}


#define pr(arg) printk(KERN_INFO arg "\n")
//#define pr(arg, ...) printk(KERN_INFO arg, __VA_ARGS__)
static int bfin_lq035_fb_open(struct fb_info *info, int user)
{
	pr("ahoj");
        return 0;
}

static int bfin_lq035_fb_release(struct fb_info *info, int user)
{
	pr("release");
        return 0;
}

static int bfin_lq035_fb_check_var(struct fb_var_screeninfo *var,
                                   struct fb_info *info)
{
	pr("check var");
        return 0;
}

/* fb_rotate
 * Rotate the display of this angle. This doesn't seems to be used by the core,
 * but as our hardware supports it, so why not implementing it...
 */
static void bfin_lq035_fb_rotate(struct fb_info *fbi, int angle)
{
	pr("rotate");
}

static int bfin_lq035_fb_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	pr("cursor");
        //if (nocursor)
                return 0;
        //else
          //      return -EINVAL; /* just to force soft_cursor() call */
}
static int bfin_lq035_fb_setcolreg(u_int regno, u_int red, u_int green,
                                   u_int blue, u_int transp,
                                   struct fb_info *info)
{
	pr("set color reg");
	return 0;
}

static struct fb_ops bfin_lq035_fb_ops = {
        .owner                  = THIS_MODULE,
        .fb_open                = bfin_lq035_fb_open,
        .fb_release             = bfin_lq035_fb_release,
        .fb_check_var           = bfin_lq035_fb_check_var,
        .fb_rotate              = bfin_lq035_fb_rotate,
        .fb_fillrect            = cfb_fillrect,
        .fb_copyarea            = cfb_copyarea,
        .fb_imageblit           = cfb_imageblit,
        .fb_cursor              = bfin_lq035_fb_cursor,
        .fb_setcolreg           = bfin_lq035_fb_setcolreg,
};


int __init probe(void)
{	
	void *fb_buffer;
	int ret = 0;

	fb_buffer = kzalloc(ACTIVE_VIDEO_MEM_SIZE, GFP_KERNEL);
	fb_info.screen_base = fb_buffer;
	fb_fix.smem_start = (long)fb_buffer;
	fb_info_defined.green.msb_right = 0;
	fb_info_defined.red.msb_right 	= 0;
	fb_info_defined.blue.msb_right	= 0;
	fb_info_defined.green.offset	= 5;
	fb_info_defined.green.length	= 6;
	fb_info_defined.red.length 	= 5;
	fb_info_defined.red.offset	= bgr ? 0 : 11;
	fb_info_defined.blue.length	= 5;
	fb_info_defined.red.offset	= bgr ? 11 : 0;
	
	fb_info.fbops	= &bfin_lq035_fb_ops;
	fb_info.var = fb_info_defined;
	
	fb_info.fix = fb_fix;
	fb_info.flags = FBINFO_DEFAULT;

	fb_info.pseudo_palette = kzalloc(sizeof(u32)*16, GFP_KERNEL);
	if (fb_info.pseudo_palette == NULL) {
		ret = -ENOMEM;
		goto out_palette;
	}

	if (fb_alloc_cmap(&fb_info.cmap, NBR_PALETTE, 0) < 0) {
		ret = -EFAULT;
		goto out_cmap;
	}

	if (register_framebuffer(&fb_info) < 0) {
		ret = -EINVAL;
		goto out_reg;
	}

	timeout.function = timer_handler;
	timeout.data = (unsigned long)fb_buffer;
	init_timer(&timeout);

	mod_timer(&timeout, jiffies + msecs_to_jiffies(500));
	pr("INIT");
	return 0;

out_reg:
	fb_dealloc_cmap(&fb_info.cmap);
out_cmap:
	kfree(&fb_info.pseudo_palette);
out_palette:
	kfree(fb_buffer);
	return ret;
}

void __exit remove(void)
{
	kfree(fb_info.pseudo_palette);
	fb_dealloc_cmap(&fb_info.cmap);
	unregister_framebuffer(&fb_info);
	//kfree(&fb_info.screen_base);
}

MODULE_LICENSE("GPL");
module_init(probe);
module_exit(remove);
