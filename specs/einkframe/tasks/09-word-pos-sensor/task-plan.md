# Task 09 — Wire word-of-the-day `pos` from HA

HA now exposes three new template sensors for the word of the day:
`sensor.word_of_the_day_pos`, `sensor.word_of_the_day_gender`,
`sensor.word_of_the_day_stuttbeyg`. Replace the hardcoded `"nafnorð"` in the
entry render with the live `pos` value.

## Scope

In:

- Add three `homeassistant` text_sensors to [sensors.package.yaml](../../../../config/packages/einkframe/sensors.package.yaml): `word_pos`, `word_gender`, `word_inflection`.
- Replace hardcoded `"nafnorð"` at [einkframe.yaml:228](../../../../config/einkframe.yaml#L228) with `pos_long(id(word_pos).state)` (helper already exists in `einkframe_utils`).
- Drop the placeholder comment above it.

Out:

- Wiring `gender` and `stuttbeyg` into the renderer — sensors are added but unused for now. Pick up in a follow-up once we decide where they belong in the layout (gender suffix on pos? inflection line?).

## Notes

- HA template returns the literal string `"None"` when the attribute is missing — guard the lambda by falling back to the existing behavior if `state == "None"` or empty.
- No new fonts, no renderer changes.

## Acceptance

- Device boots, `word_pos` populates from HA, the rendered word class matches the real `pos` attribute (e.g. "nafnorð", "sagnorð", "lýsingarorð") instead of always "nafnorð".
