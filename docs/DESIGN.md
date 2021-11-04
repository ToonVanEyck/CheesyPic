# Creating a custom photostrip design. #        {#design}

The easiest way to create a custom design is to start from the `template.design.svg` located in the `assets` directory.
This file can be easly modified by using the free vector graphic editor [inkscape](https://inkscape.org/), or any vector graphic edditor you like.

The design template has a size of 6 inches by 4 inches. This corrsponds with the most common photo size. To accomodete for differend sized media, please resize the template to the required size.

## Placeholders ##

The design template contains some special placeholder objects, these paceholders will be replaced with the actual photos that the photobooth captures. The cheesypic software will automaticaly determine the amount of photos in the design based on these placeholders.

A placeholder is defined as folows:
```
<g cheesypic_placeholder="true">
    <image/>
    <text>2</text>
    <...>
</g>
```
A placeholder is an svg group with a ```cheesypic_placeholder``` attribute set to true. This group must contain **only one image tag**, **only one text tag** and any other tags like rects, paths and groups. The image tag does not have to contain any image data, the image data is placed there at the time of printing. The text tag defines the photo number. E.g.: 1 means the first captured photo will be placed on this location. When the time comes to print the photo, all tags in this group are removed except for the image.

Note that the design file defines a full sized photo. This full sized photo is cut in half when the printer is setup to print photostrips.
