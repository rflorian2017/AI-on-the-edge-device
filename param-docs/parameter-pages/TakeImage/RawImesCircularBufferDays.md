# Parameter `RawImesCircularBufferDays`

Number of day-slots used by the circular-buffer image storage.

Each slot stores all raw images captured on one calendar day.  After
`RawImesCircularBufferDays` have been filled, the oldest slot is
cleared and reused.

Only effective when `RawImesCircularBuffer = true`.

Unit: Days

Range: 1–30

Default Value: `30`
