# httpng

A web thumbnailer written in C.

## Usage

```
Usage:
  httpng [OPTION...] url filename.png

Take a thumbnail of a website.

Help Options:
  -?, --help               Show help options
  --help-all               Show all help options
  --help-gtk               Show GTK+ Options

Application Options:
  -i, --images             Load images
  -f, --favicon            Add favicon
  -w, --width              Image width (Default: 800)
  -h, --height             Image height (Default: 600)
  --dwidth                 Device width (Default: 800)
  --dheight                Device height (Default: 600)
  -z, --zoom               Zoom level (25-500)% (Default: 100)
  -x, --xpan               Horizontal panning (Default: 0)
  -y, --ypan               Veritical panning (Default: 0)
  -u, --user-agent         User agent string
  --display=DISPLAY        X display to use
```

## Copyright

Copyright (c) 2012 Scott LaVigne. See [LICENSE.txt][license] for details.

[license]: https://github.com/pyrated/httpng/blob/master/LICENSE.txt
