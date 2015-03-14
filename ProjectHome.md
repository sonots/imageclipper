# Introduction #

**(2014/05/20) The maintainer was changed to [Joakim Soderberg](https://github.com/JoakimSoderberg). Go to https://github.com/JoakimSoderberg/imageclipper**

It is often required to crop images manually fast for computer vision researchers to gather training and testing image sets, e.g., cropping faces in images.

This simple multi-platform (Windows and Linux were verified) software helps you to clip images manually fast.

## What You Can Do ##

  * Open images in a directory **sequentially**
  * Open a **video**, frame by frame
  * Clip (save) and go to the next image by **one button** (SPACE)
  * Move and resize your selected region by **vi-like hotkeys** or **right mouse button**
  * **Rotate and shear deform (affine transform)** the rectangle region

# Table of Contents #

  * [Download](#Download.md)
  * [Application Usage](#Application_Usage.md)
  * [Command Usage](#Command_Usage.md)
  * [How to Compile on Linux](#How_to_Compile_on_Linux.md)
  * [How to Compile on Windows](#How_to_Compile_on_Windows.md)
  * [Creating a text file to locate clipped regions](#Creating_a_text_file_to_locate_clipped_region.md)
  * Leave comments at [CommentPage](CommentPage.md)

# Download #

Windows binary and source codes
  * http://code.google.com/p/imageclipper/downloads/list

Drag and Drop a folder (including pictures) or a picture or a video file on exe.
You can also execute it on the command prompt.
No installer is included for this simple software.

PS. It may be required to install [Microsoft Visual C++ 2005 SP1 Redistributable Package](http://www.microsoft.com/downloads/details.aspx?FamilyID=200b2fd9-ae1a-4a14-984d-389c36f85647&DisplayLang=en) (if you have never installed this or Visual Studio 2005 itself). I am looking for how not to require users to install it. Not to use VC++ 2005?

How to compile for Linux is written below.

# Application Usage #

(Windows) Drag and drop a folder or an image file or a video file on imageclipper.exe.

(Windows and Linux) Execute as a command line tool. See [Command Usage](#Command_Usage.md) section too.

![http://imageclipper.googlecode.com/files/snapshot.png](http://imageclipper.googlecode.com/files/snapshot.png)

## Mouse Usage ##
```
    Left  (select)          : Select or initialize a rectangle region.
    Right (move or resize)  : Move by dragging inside the rectangle.
                            : Resize by draggin outside the rectangle.
```

## Keyboard Usage ##

```
    s (save)                : Save the selected region as an image.
    f (forward)             : Forward. Show next image.
    SPACE                   : Save and Forward.
    b (backward)            : Backward. 
    q (quit) or ESC         : Quit.
    r (rotate) R (counter)  : Rotate rectangle in clockwise.
    e (expand) E (shrink)   : Expand the recntagle size.
    h (left) j (down) k (up) l (right) : Move rectangle. (vi-like keybinds)
    y (left) u (down) i (up) o (right) : Resize rectangle (Move right-bottom boundaries).
    n (left) m (down) , (up) . (right) : Shear deformation.
```

## Saved file format ##

The clipped images are saved as
```
<dirname>/imageclipper/<basename>_<rotation_degree>_<left_x>_<top_y>_<width>_<height>.png
```
such as
```
../myimages/imageclipper/lena.jpg_0359_0109_0093_0065_0090.png
```
where the original filename is
```
../myimages/lena.jpg. 
```

A directory named "imageclipper" is created under the image file's directory and cropped images are stored on there.

This filename format can be configured via command line options. See [Command Usage](#Command_Usage.md) section.

## Experimental Feature (Continuous Color Region) ##

You can choose a rectangular region surrounding a continuous color region.


```
    Middle or SHIFT + Left  : Initialize the watershed marker. Drag it.
```

You can use Right mouse and hotkeys to move or resize the watershed marker.

![http://imageclipper.googlecode.com/files/watershed.png](http://imageclipper.googlecode.com/files/watershed.png)

# Command Usage #

```

ImageClipper - image clipping helper tool.
Command Usage: imageclipper.exe [option]... [reference]
  <reference = .>
    <reference> would be a directory or an image or a video filename.
    For a directory, image files in the directory will be read sequentially.
    For an image (a file having supported image type extensions listed below),
    it starts to read a directory from the specified image file.
    For a video (a file except images), the video is read.

  Options
    -f <output_format = imgout_format or vidout_format>
        Determine the output file path format.
        This is a syntax sugar for -i and -v.
        Format Expression)
            %d - dirname of the original
            %i - filename of the original without extension
            %e - filename extension of the original
            %x - upper-left x coord
            %y - upper-left y coord
            %w - width
            %h - height
            %r - rotation degree
            %. - shear deformation in x coord
            %, - shear deformation in y coord
            %f - frame number (for video)
        Example) ./$i_%04x_%04y_%04w_%04h.%e
            Store into software directory and use image type of the original.
    -i <imgout_format = %d/imageclipper/%i.%e_%04r_%04x_%04y_%04w_%04h.png>
        Determine the output file path format for image inputs.
    -v <vidout_format = %d/imageclipper/%i.%e_%04f_%04r_%04x_%04y_%04w_%04h.png>
        Determine the output file path format for a video input.
    -h
    --help
        Show this help

  Supported Image Types
      bmp|dib|jpeg|jpg|jpe|png|pbm|pgm|ppm|sr|ras|tiff|exr|jp2
```

You may make a batch file (Windows) or a shell script or an alias (Linux) to setup default values. Use of -i and -v rather than -f would be useful in that case.

# How to Compile on Linux #


You need to install [OpenCV](http://sourceforge.net/projects/opencvlibrary/) and [Boost](http://www.boost.org) libraries.

## Install OpenCV ##

[Download OpenCV](http://sourceforge.net/project/showfiles.php?group_id=22870&package_id=16948&release_id=461516) source codes (.tar.gz)

root user
```
tar zxvf opencv-1.0.0.tar.gz; cd opencv-1.0.0
./configure
make
make install
```

general user
```
tar zxvf opencv-1.0.0.tar.gz; cd opencv-1.0.0
./configure --prefix=$HOME/usr
make
make install
```

```
export PATH=$HOME/usr/bin/:$PATH
export LD_LIBRARY_PATH=$HOME/usr/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$HOME/usr/lib/pkgconfig:$PKG_CONFIG_PATH
export MANPATH=$HOME/usr/man:$MANPATH
```


## Install Boost ##

root user and Red Hat (Fedora)
```
yum install boost
yum install boost-dev
```

root user and Debian (Ubunts)
```
apt-get install boost
apt-get install boost-dev
```

general user

[Download Boost](http://sourceforge.net/project/showfiles.php?group_id=7586&package_id=8041) source codes (.tar.gz)

```
tar zxvf boost_1_36_0.tar.gz; cd boost_1_36_0
./configure --prefix=$HOME/usr
make install
```

Reference: [Boost Getting Started on Unix Variants](http://www.boost.org/doc/libs/1_36_0/more/getting_started/unix-variants.html)

## Compile ##

Unarchive imageclipper.zip. Go to src/ directory.
A Makefile is attached. Modify as follows:

  1. If you installed boost not on $HOME/usr/: modify ~/usr/ to your path such as /usr/
  1. If your boost version is not boost\_1\_36\_0: modify boost-1\_36 to your version.
  1. Modify -lboost\_system-gcc41-mt -lboost\_filesystem-gcc41-mt to yours. Find names by $ ls /path/to/yourboost/lib. If you find names as libboost\_filesystem-gcc41-mt, then => -lboost\_filesystem-gcc41-mt
  1. $ make check

I don't know why boost library uses different names under different system.
It requires us to modify Makefile as above in different system.... anyway,

Finally
```
make
```

You should get an executable file "imageclipper".


# How to Compile on Windows #

A binary file for Windows is already attached.
This section explains how to compile for those who want to modify source codes and create a modified software.

You need to install [OpenCV](http://sourceforge.net/projects/opencvlibrary/) and [Boost](http://www.boost.org) libraries.

## Install OpenCV ##


[Download OpenCV](http://sourceforge.net/project/showfiles.php?group_id=22870&package_id=16937&release_id=456897) installer and install it. I assume you have installed onto C:\Program Files\OpenCV.

## Install Boost ##

[Download boost](http://www.boostpro.com/products/free) installer and install it. I assume you have installed onto C:\Program Files\boost\boost\_1\_35\_0.

Reference: [Boost Getting Started on Windows](http://www.boost.org/doc/libs/1_36_0/more/getting_started/windows.html)

## Setup MS Visual C++ ##

I write details based on MS Visual Studio 2005.
Change boost\_1\_35\_0 if your version is different.
Follow menus of MS Visual C++ window as:

  * Tools > Options > Projects and Solutions > VC++ directories >
    * Show Directory for: > Include Files. Now add
```
C:\Program Files\boost\boost_1_35_0
C:\Program Files\OpenCV\cv\include
C:\Program Files\OpenCV\cvaux\include
C:\Program Files\OpenCV\cxcore\include
C:\Program Files\OpenCV\otherlibs\highgui
```
    * Show Directory for: > Library Files. Now add
```
C:\Program Files\boost\boost_1_35_0\lib
C:\Program Files\OpenCV\lib
```

## Compile ##

Unarchive imageclipper.zip and open src\imageclipper.sln, then Build for Release.

# Creating a text file to locate clipped region #

A text file containing locations of clipped region is sometimes required rather than clipped images themselves, e.g., as a ground truth text for testing experiments in computer vision.

Assume there exist files as follows:
```
$ \ls imageclipper/*_*_*
image.jpg_0068_0047_0089_0101.png
image.jpg_0087_0066_0090_0080.png
image.jpg_0095_0105_0033_0032.png
image.jpg_0109_0093_0065_0090.png
image.jpg_0117_0097_0052_0095.png
```
By executing the following command,
```
$ find imageclipper/*_*_* -exec basename \{\} \; | perl -pe \
's/([^_]*).*_0*(\d+)_0*(\d+)_0*(\d+)_0*(\d+)\.[^.]*$/$1 $2 $3 $4 $5\n/g' \
| tee clipping.txt
```
you can obtain a text file "clipping.txt" as follows:
```
image.jpg 68 47 89 101
image.jpg 87 66 90 80
image.jpg 95 105 33 32
image.jpg 109 93 65 90
image.jpg 117 97 52 95
```

I think this file format is a typical format for testing experiments in many computer vision tools.

## For OpenCV haartraing Users ##

Use [haartrainingformat.pl](http://imageclipper.googlecode.com/svn/trunk/haartrainingformat.pl) as
```
$ perl haartrainingformat.pl clipping.txt | tee info.dat
```
where clipping.txt is the text file created at the previous section to create a OpenCV haartraining suitable format such as
```
image.jpg 5 68 47 89 101 87 66 90 80 95 105 33 32 109 93 65 90 117 97 52 95
```

Or you can generate a file without creating "clipping.txt" as
```
$ \ls imageclipper/*_*_* | perl haartrainingformat.pl --ls --trim --basename | tee info.dat
```

Note that clipped images themselves are enough and better for training phase
(See [Tutorial: OpenCV haartraining](http://note.sonots.com/SciSoftware/haartraining.html#o40a43fd)).
Therefore, you may use this only for creation of testing data.

Usage
```
Helper tool to convert a specific format into a haartraining format.
Usage: perl haartrainingformat.pl [option]... [filename]
    filename
        file to be read. STDIN is also supported.
Options:
    -l
    --ls
        input is an output of ls.
    -t
    --trim
        trim heading 0 of numbers like from 00xx to xx.
    -b
    --basename
        get basenames from filenames.
    -h
    --help
        show this help.
```