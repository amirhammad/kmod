#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/spi/spi.h>
/*
static int ad714x_spi_write(struct ad714x_chip *chip,
                            unsigned short reg, unsigned short data)
{
        struct spi_device *spi = to_spi_device(chip->dev);
        int error;

        //chip->xfer_buf[0] = cpu_to_be16(AD714x_SPI_CMD_PREFIX | reg);
        //chip->xfer_buf[1] = cpu_to_be16(data);

        error = spi_write(spi, (u8 *)chip->xfer_buf,
                          2 * sizeof(*chip->xfer_buf));
        if (unlikely(error)) {
                dev_err(chip->dev, "SPI write error: %d\n", error);
                return error;
        }

        return 0;
}
*/

static int ad714x_spi_probe(struct spi_device *spi)
{
	int err;
        spi->bits_per_word = 8;
        //err = spi_setup(spi);
        if (err < 0)
                return err;
/*
        struct ad714x_chip *chip;
        int err;

        spi->bits_per_word = 8;
        err = spi_setup(spi);
        if (err < 0)
                return err;

        chip = ad714x_probe(&spi->dev, BUS_SPI, spi->irq,
                            ad714x_spi_read, ad714x_spi_write);
        if (IS_ERR(chip))
                return PTR_ERR(chip);
*/
        spi_set_drvdata(spi, NULL);

        return 0;
}

struct mydev {
	int a;
};
static int ad714x_spi_remove(struct spi_device *spi)
{
        struct mydev *drvdata = spi_get_drvdata(spi);

        return 0;
}

static struct spi_driver ad714x_spi_driver = { 
        .driver = { 
                .name   = "Tspi",
                .owner  = THIS_MODULE,
        },
        .probe          = ad714x_spi_probe,
        .remove         = ad714x_spi_remove,
};

module_spi_driver(ad714x_spi_driver);
/*
static int __init probe(void)
{
	spi_driver_register(&ad714x_spi_driver);
}
static void __exit remove(void)
{
	
	spi_driver_unregister(&ad714x_spi_driver);
}

module_init(probe);
module_exit(remove);
*/
