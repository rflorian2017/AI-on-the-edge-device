# Parameter `HtmlStorage`
Default Value: `SDCard`

Select where the Web UI files are served from.

Possible values:

- `SDCard` (default)
- `Internal`

Set to `Internal` to use the Web UI files from the internal `webui` LittleFS partition (mounted at `/html`) instead of `/sdcard/html`.
