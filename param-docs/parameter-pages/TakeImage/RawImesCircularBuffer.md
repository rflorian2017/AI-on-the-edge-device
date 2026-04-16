# Parameter `RawImesCircularBuffer`

Enable the circular-buffer mode for raw (source) images.

When set to `true`, images are stored in numbered day-slots (`cbuf_00` …
`cbuf_NN`) under the configured `RawImagesLocation`.  Once all slots are
filled, the oldest slot is reused automatically, providing a fixed,
bounded storage footprint without manual clean-up.

Set `RawImesCircularBufferDays` to control how many day-slots are kept.

Default Value: `false`
