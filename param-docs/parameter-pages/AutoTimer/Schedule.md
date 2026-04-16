# Parameter `Schedule`
Default Value: ``

Optional daily wakeup schedule in local time (`HH:MM`), separated by `;` or `,`.

Examples:

- `10:00` (once per day)
- `00:00;05:00;10:00;15:00;20:00` (every 5 hours)

If this parameter is empty, the regular `Interval` timing is used.
