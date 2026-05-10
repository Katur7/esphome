/* global React, SAMPLE_DATA, Icon */

// =============== Variant 2 — Swiss Poster ===============
// Asymmetric grid, hard rules, oversize sans for the word.
// Date is the kicker, weather is a rail, examples are pinned to the right column.
function VariantSwiss({ data, showBattery = true }) {
  return (
    <div className="eink v-swiss">
      <div className="top">
        <div className="stamp">EINK<br/>FRAME</div>
        <div className="title">
          <div className="kicker">Dagens datum</div>
          <div className="date">{data.dateLong}</div>
        </div>
        <div className="wxcell">
          <Icon name={data.weather} style={{fontSize: 48}} />
          <div className="temp">{data.temperature}°</div>
        </div>
      </div>

      <div className="hero">
        <div className="word-cell">
          <div className="word-num">№ 01 / ORÐ</div>
          <div>
            <div className="label">Orð dagsins · Íslenska</div>
            <div className="word">{data.word}</div>
            <div className="translations">{data.translations}</div>
          </div>
        </div>

        <div className="ex-cell">
          <div className="label">Exempel · Användning</div>
          {data.examples.map((ex, i) => (
            <div key={i}>
              <div className="ex ex-is">{ex.is}</div>
              <div className="ex ex-se">{ex.se}</div>
            </div>
          ))}
        </div>
      </div>

      <div className="strip">
        <div className="cell">
          <div className="lab">Regn</div>
          <div className="val">
            <Icon name="umbrella" style={{fontSize: 22}} />
            <span>{Math.round(parseFloat(data.expectedRain))}<span className="unit">%</span></span>
          </div>
        </div>
        <div className="cell">
          <div className="lab">UV-index</div>
          <div className="val">
            <Icon name="uv" style={{fontSize: 22}} />
            <span>{parseFloat(data.uvIndex).toFixed(1)}</span>
          </div>
        </div>
        <div className="cell">
          <div className="lab">Väder</div>
          <div className="val">
            <Icon name={data.weather} style={{fontSize: 22}} />
            <span style={{fontSize: 14, fontWeight: 800, letterSpacing: '0.04em'}}>
              {weatherLabel(data.weather)}
            </span>
          </div>
        </div>
        {showBattery ? (
          <div className="cell">
            <div className="lab">Batteri</div>
            <div className="val">
              <Icon name="battery" style={{fontSize: 22}} />
              <span>{Math.round(data.batteryPercent)}<span className="unit">%</span></span>
            </div>
          </div>
        ) : (
          <div className="cell">
            <div className="lab">Uppdaterad</div>
            <div className="val">
              <span style={{fontSize: 22, fontWeight: 900}}>15:00</span>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

function weatherLabel(state) {
  return ({
    "clear-night":     "klart",
    "cloudy":          "molnigt",
    "fog":             "dimma",
    "hail":            "hagel",
    "lightning":       "åska",
    "lightning-rainy": "åskregn",
    "partlycloudy":    "växlande",
    "pouring":         "ösregn",
    "rainy":           "regn",
    "snowy":           "snö",
    "snowy-rainy":     "snöblandat",
    "sunny":           "soligt",
    "windy":           "blåsigt",
    "windy-variant":   "kraftig vind",
    "exceptional":     "extremt",
  })[state] || state;
}

window.VariantSwiss = VariantSwiss;
window.weatherLabel = weatherLabel;
