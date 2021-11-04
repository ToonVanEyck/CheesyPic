# Configuration #        {#configuration}

The CheesyPic photobooth software contains some configurable parameters. These can be configured by altering the ````cheesypic.conf``` file. This file is located at ```usr/local/etc/cheesypic.conf```

## design ##

### design_directory ###

This option defines the path where the photoboot will look for a design file. Only files ending in ```.design.svg``` are considered.

### force_design ###

By default, the software uses the most recently modified design file in the design_directory. Setting this option forces the software to always use the defined design from the design_directory.

## theme ##

### theme_directory ###

This option defines the path where the photoboot will look for a theme file. Only files ending in ```.theme.svg``` are considered.

### force_theme ###

By default, the software uses the most recently modified theme file in the theme_directory. Setting this option forces the software to always use the defined theme from the theme_directory.
