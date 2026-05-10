/* global React, Icon, weatherLabel */
// 07 — Orð dagsins harmony
// Brings the web project's editorial vocabulary into the e-ink layout.
//
// Signature elements ported from handoff 3:
//   • Folio header pattern        — small-caps Inter under an ink hairline
//   • Three peg labels            — Íslenska / Svenska / Dæmi
//   • Newsreader serif            — replaces Fraunces in this variant
//   • Spelled-out word class      — "lýsingarorð" italic, not "lo."
//   • Inflection line             — italic, smaller (optional)
//   • Dotted translation list     — "·" separators, items nowrap
//   • Hairlines between examples  — vertical rule + per-example hairlines
//
// Oxblood (#7a1a22) collapses to black in 1-bit; "muted" collapses to italic + smaller scale.

// ---- helpers ----
function posLong(pos) {
  if (!pos) return "";
  const m = String(pos).match(/\(([^)]+)\)/);
  return m ? m[1] : pos;
}

function splitTranslations(s) {
  return String(s || "")
    .split(/\s*[,·]\s*/)
    .map((t) => t.trim())
    .filter(Boolean);
}

// Shrink-to-fit headword: simple JS measurement, runs after fonts are ready.
function useFitHeadword(ref, { min, max }) {
  React.useEffect(() => {
    const el = ref.current;
    if (!el) return;
    const measure = el.querySelector("[data-ord-measure]");
    const visible = el.querySelector("[data-ord-visible]");
    if (!measure || !visible) return;

    const fit = () => {
      const available = el.clientWidth;
      let lo = min, hi = max, best = min;
      measure.style.fontSize = max + "px";
      if (measure.offsetWidth <= available) {
        visible.style.fontSize = max + "px";
        visible.style.whiteSpace = "nowrap";
        return;
      }
      measure.style.fontSize = min + "px";
      if (measure.offsetWidth > available) {
        visible.style.fontSize = min + "px";
        visible.style.whiteSpace = "normal";
        visible.style.wordBreak = "break-word";
        return;
      }
      for (let i = 0; i < 18; i++) {
        const mid = (lo + hi) / 2;
        measure.style.fontSize = mid + "px";
        if (measure.offsetWidth <= available) { best = mid; lo = mid; }
        else { hi = mid; }
      }
      visible.style.fontSize = Math.floor(best) + "px";
      visible.style.whiteSpace = "nowrap";
    };

    fit();
    if (document.fonts && document.fonts.ready) document.fonts.ready.then(fit);
    const ro = new ResizeObserver(fit);
    ro.observe(el);
    return () => ro.disconnect();
  }, [ref, min, max]);
}

// ---- Peg label ----
function Peg({ children, className = "" }) {
  return (
    <div className={`ord-peg ${className}`}>
      <span className="ord-peg-sq" />
      <span className="ord-peg-lbl">{children}</span>
    </div>
  );
}

// ---- Translations as a dotted list ----
// Caps at MAX_VISIBLE items to keep the hero column from overflowing into the
// info strip on extreme entries (corpus max is 28). Overflow is summarised
// with a tracked Inter "+N fler" tag so the reader knows there's more.
const MAX_VISIBLE_TRANSLATIONS = 15;

function DottedTranslations({ value }) {
  const items = splitTranslations(value);
  const visible = items.slice(0, MAX_VISIBLE_TRANSLATIONS);
  const overflow = items.length - visible.length;
  return (
    <p className="ord-trans" lang="sv">
      {visible.map((t, i) => (
        <React.Fragment key={i}>
          {i > 0 && <span className="ord-sep">·</span>}
          <span className="ord-t">{t}</span>
        </React.Fragment>
      ))}
      {overflow > 0 ? (
        <>
          <span className="ord-sep">·</span>
          <span className="ord-trans-more">+{overflow} fler</span>
        </>
      ) : null}
    </p>
  );
}

// ---- Bottom info strip (shared with Final A — same 4-cell strip) ----
function OrdStrip({ data, showBattery }) {
  return (
    <div className="hyb-strip">
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
          <span className="wx-word">{weatherLabel(data.weather)}</span>
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
          <div className="val"><span style={{fontSize: 22, fontWeight: 900}}>15:00</span></div>
        </div>
      )}
    </div>
  );
}

// =========== 07a — Faithful port: folio + three pegs + Newsreader ===========
function VariantOrdA({ data, showBattery = true }) {
  const heroRef = React.useRef(null);
  useFitHeadword(heroRef, { min: 32, max: 72 });

  const inflections = data.inflections;

  return (
    <div className="eink v-ord v-ord-a">
      {/* Folio header */}
      <header className="ord-folio">
        <div className="ord-folio-left">
          <span className="ord-folio-title">Orð dagsins</span>
          <span className="ord-folio-mid">·</span>
          <span className="ord-folio-date">{data.dateLong}</span>
        </div>
        <div className="ord-folio-wx">
          <Icon name={data.weather} style={{fontSize: 32}} />
          <span className="ord-folio-temp">{data.temperature}°</span>
        </div>
      </header>

      {/* Body */}
      <main className="ord-body">
        <section className="ord-hero">
          <Peg className="ord-peg-is">Íslenska</Peg>

          <div className="ord-headword-fit" ref={heroRef}>
            <span className="ord-headword-measure" aria-hidden="true" data-ord-measure>{data.word}</span>
            <h1 className="ord-headword" data-ord-visible>{data.word}</h1>
          </div>

          <p className="ord-class">{posLong(data.pos)}</p>
          {inflections ? (
            <p className="ord-infl">{inflections}</p>
          ) : null}

          <div className="ord-se-block">
            <Peg className="ord-peg-se">Svenska</Peg>
            <DottedTranslations value={data.translations} />
          </div>
        </section>

        <section className="ord-examples-col">
          <Peg className="ord-peg-daemi">Dæmi</Peg>
          <ol className="ord-example-list">
            {data.examples.map((ex, i) => (
              <li className="ord-example" key={i}>
                <p className="ord-ex-is">{ex.is}</p>
                <p className="ord-ex-se" lang="sv">{ex.se}</p>
              </li>
            ))}
          </ol>
        </section>
      </main>

      <OrdStrip data={data} showBattery={showBattery} />
    </div>
  );
}

// =========== 07b — Light touch: Final-A header + Newsreader + pegs ===========
// Keeps the existing Swiss-block header (date + weather) and 4-cell strip;
// swaps just the entry body to the editorial vocabulary.
function VariantOrdB({ data, showBattery = true }) {
  const heroRef = React.useRef(null);
  useFitHeadword(heroRef, { min: 36, max: 62 });

  return (
    <div className="eink v-ord v-ord-b">
      {/* Final-A style top — kept for renderer parity */}
      <div className="hyb-top">
        <div className="title"><div className="date">{data.dateLong}</div></div>
        <div className="wxcell">
          <Icon name={data.weather} style={{fontSize: 48}} />
          <div className="temp">{data.temperature}°</div>
        </div>
      </div>

      <main className="ord-body ord-body-b">
        <section className="ord-hero">
          <Peg className="ord-peg-is">Íslenska</Peg>
          <div className="ord-headword-fit" ref={heroRef}>
            <span className="ord-headword-measure" aria-hidden="true" data-ord-measure>{data.word}</span>
            <h1 className="ord-headword" data-ord-visible>{data.word}</h1>
          </div>
          <p className="ord-class">{posLong(data.pos)}</p>

          <div className="ord-se-block">
            <Peg className="ord-peg-se">Svenska</Peg>
            <DottedTranslations value={data.translations} />
          </div>
        </section>

        <section className="ord-examples-col">
          <Peg className="ord-peg-daemi">Dæmi</Peg>
          <ol className="ord-example-list">
            {data.examples.map((ex, i) => (
              <li className="ord-example" key={i}>
                <p className="ord-ex-is">{ex.is}</p>
                <p className="ord-ex-se" lang="sv">{ex.se}</p>
              </li>
            ))}
          </ol>
        </section>
      </main>

      <OrdStrip data={data} showBattery={showBattery} />
    </div>
  );
}

window.VariantOrdA = VariantOrdA;
window.VariantOrdB = VariantOrdB;
