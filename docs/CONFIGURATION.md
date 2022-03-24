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

## save ##

### save_photos ###

Enable this option to save all captured photos.

### save_directory ###

This is where the photos are stored when save_photos is enabled. A directory with the same name as the design will be created to hold all the saved photos.

## addons ##

### addon_script ###

The ```addon_script``` will be executed by forking from the main application and will receive the PID of the cheesypic application as an argument.

## advanced ##

### countdown_time ###

This option defines the delay between each increment of the countdown in seconds. (deafult 1s)

### preview_time ###

This option defines how long each photo is shown on the screen after it is taken.

### mirror_liveview ###

This option causes the liveview to be mirrored. This alows users to more easely possition themself before a photobooth.

### mirror_preview ###

Enableing this option will cause the the preview of a captured photo to appear mirrored on the screen.

### windowless_mode ###

This option forces the cheesypic software in fulscreen borderless mode.
