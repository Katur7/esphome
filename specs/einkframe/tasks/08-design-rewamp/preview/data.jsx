/* global React */
// Shared sample data for all variants. Real device pulls these from HA.
const SAMPLE_DATA = {
  date: "Tisdag, 5 maj",
  dateLong: "Tisdag 5 maj 2026",
  day: "5",
  weekday: "Tisdag",
  month: "Maj",
  year: "2026",
  weather: "partlycloudy",
  temperature: "12",
  expectedRain: "23",
  uvIndex: "4.2",
  batteryPercent: 78,
  word: "sjálfsmorðshugleiðingar",
  pos: "no. (nafnorð)",
  translations: "slita som ett djur, arbeta, jobba, åstadkomma, utföra, vålla, bearbeta, vinna, segra, förvärvsarbeta, arbeta på <uppgiften>, vinna i längden, ta sig, försörja <sig>, motarbeta <det allmänna bästa>, tjäna pengar, jobba på <problemet>, vinna <ett pris>, jobba ikapp <förseningen>, jobba sig upp, utvinna <medicin> ur <växten>, bearbeta <data>, vinna en seger, arbeta mot <betalning>, arbeta med att rensa fisk, jobba med att rensa fisk, bearbeta <traumat>, bryta <silver> ur berget",
  examples: [
    {
      is: "hann leit ekki á sjálfan sig sem atvinnusöngvara þótt hann syngi oft á mannamótum",
      se: "han såg inte sig själv som någon professionell sångare även om han ofta sjöng offentligt",
    },
    {
      is: "það segir sig sjálft að ekki er hægt að reka fyrirtækið lengi með tapi",
      se: "det säger sig självt att det inte går att driva företaget någon längre tid när det går med förlust",
    },
  ],
};

// Material Design Icons codepoints — we use the @mdi/font web font.
// Simply rendering the codepoint character with font-family: "Material Design Icons".
const MDI = {
  "clear-night":     "\u{F0594}",
  "cloudy":          "\u{F0590}",
  "exceptional":     "\u{F05D6}",
  "fog":             "\u{F0591}",
  "hail":            "\u{F0592}",
  "lightning":       "\u{F0593}",
  "lightning-rainy": "\u{F067E}",
  "partlycloudy":    "\u{F0595}",
  "pouring":         "\u{F0596}",
  "rainy":           "\u{F0597}",
  "snowy":           "\u{F0598}",
  "snowy-rainy":     "\u{F067F}",
  "sunny":           "\u{F0599}",
  "windy":           "\u{F059D}",
  "windy-variant":   "\u{F059E}",
  "umbrella":        "\u{F054A}",
  "cloud-percent":   "\u{F1A35}",
  "uv":              "\u{F17FF}",
  "battery":         "\u{F0079}",
};

const Icon = ({ name, style }) => (
  <span className="mdi" style={style}>{MDI[name] || ""}</span>
);

window.SAMPLE_DATA = SAMPLE_DATA;
window.MDI = MDI;
window.Icon = Icon;
