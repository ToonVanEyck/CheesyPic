# Configuration #        {#configuration}

The CheesyPic photobooth software contains some configurable parameters. These can be configured by altering the ````cheesypic.conf``` file. This file is located at ```usr/local/etc/cheesypic.conf```

## design ##

### design_directory ###

This option defines the path where the photoboot will look for a design file. Only files ending in ```.design.svg``` are considered. By default, the latest design in the design directory is used.

### force_design ###

Setting this option will force the defined design to be used instead of the most recent one.

## theme ##

### theme_directory ###

This option defines the path where the photoboot will look for a theme file. Only files ending in ```.theme.svg``` are considered. By default, the latest theme in the theme directory is used.

### force_theme ###

Setting this optionwill force the defined theme to be used instead of the most recent one.
