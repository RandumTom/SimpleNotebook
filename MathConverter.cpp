#include "MathConverter.h"
#include <QString>
#include <QMap>

// Helper to convert a digit string to superscript
static QString superscriptDigit(const QString &num)
{
    static const QMap<QChar, QChar> superscripts = {
        {QChar('0'), QChar(8304)},  // ⁰
        {QChar('1'), QChar(185)},   // ¹
        {QChar('2'), QChar(178)},   // ²
        {QChar('3'), QChar(179)},   // ³
        {QChar('4'), QChar(8308)},  // ⁴
        {QChar('5'), QChar(8309)},  // ⁵
        {QChar('6'), QChar(8310)},  // ⁶
        {QChar('7'), QChar(8311)},  // ⁷
        {QChar('8'), QChar(8312)},  // ⁸
        {QChar('9'), QChar(8313)},  // ⁹
        {QChar('+'), QChar(8314)},  // ⁺
        {QChar('-'), QChar(8315)},  // ⁻
        {QChar('n'), QChar(8317)},  // ⁿ
        {QChar('i'), QChar(7523)},  // ⁱ
    };
    
    QString result;
    for (const QChar &c : num) {
        if (superscripts.contains(c)) {
            result.append(superscripts[c]);
        } else {
            return QString(); // Can't convert
        }
    }
    return result;
}

// Map of LaTeX input -> Unicode output
static const QMap<QString, QString> &getMathMap()
{
    static QMap<QString, QString> map;
    if (map.isEmpty()) {
        // Greek letters (lowercase)
        map["alpha"] = "α";
        map["beta"] = "β";
        map["gamma"] = "γ";
        map["delta"] = "δ";
        map["epsilon"] = "ε";
        map["zeta"] = "ζ";
        map["eta"] = "η";
        map["theta"] = "θ";
        map["iota"] = "ι";
        map["kappa"] = "κ";
        map["lambda"] = "λ";
        map["mu"] = "μ";
        map["nu"] = "ν";
        map["xi"] = "ξ";
        map["omicron"] = "ο";
        map["pi"] = "π";
        map["rho"] = "ρ";
        map["sigma"] = "σ";
        map["tau"] = "τ";
        map["upsilon"] = "υ";
        map["phi"] = "φ";
        map["chi"] = "χ";
        map["psi"] = "ψ";
        map["omega"] = "ω";
        
        // Greek letters (uppercase)
        map["Gamma"] = "Γ";
        map["Delta"] = "Δ";
        map["Theta"] = "Θ";
        map["Lambda"] = "Λ";
        map["Xi"] = "Ξ";
        map["Pi"] = "Π";
        map["Sigma"] = "Σ";
        map["Upsilon"] = "Υ";
        map["Phi"] = "Φ";
        map["Psi"] = "Ψ";
        map["Omega"] = "Ω";
        
        // Superscripts
        map["^0"] = "⁰";
        map["^1"] = "¹";
        map["^2"] = "²";
        map["^3"] = "³";
        map["^4"] = "⁴";
        map["^5"] = "⁵";
        map["^6"] = "⁶";
        map["^7"] = "⁷";
        map["^8"] = "⁸";
        map["^9"] = "⁹";
        map["^+"] = "⁺";
        map["^-"] = "⁻";
        map["^="] = "⁼";
        map["^("] = "⁽";
        map["^)"] = "⁾";
        map["^n"] = "ⁿ";
        map["^i"] = "ⁱ";
        
        // Subscripts
        map["_0"] = "₀";
        map["_1"] = "₁";
        map["_2"] = "₂";
        map["_3"] = "₃";
        map["_4"] = "₄";
        map["_5"] = "₅";
        map["_6"] = "₆";
        map["_7"] = "₇";
        map["_8"] = "₈";
        map["_9"] = "₉";
        map["_+"] = "₊";
        map["_-"] = "₋";
        map["_="] = "₌";
        map["_("] = "₍";
        map["_)"] = "₎";
        map["_a"] = "ₐ";
        map["_e"] = "ₑ";
        map["_h"] = "ₕ";
        map["_i"] = "ᵢ";
        map["_j"] = "ⱼ";
        map["_k"] = "ₖ";
        map["_l"] = "ₗ";
        map["_m"] = "ₘ";
        map["_n"] = "ₙ";
        map["_o"] = "ₒ";
        map["_p"] = "ₚ";
        map["_r"] = "ᵣ";
        map["_s"] = "ₛ";
        map["_t"] = "ₜ";
        map["_u"] = "ᵤ";
        map["_v"] = "ᵥ";
        map["_x"] = "ₓ";
        map["_y"] = "ᵧ";
        map["_z"] = "ᵨ";
        
        // Math operators
        map["sqrt"] = "√";
        map["sum"] = "Σ";
        map["prod"] = "Π";
        map["product"] = "Π";
        map["integral"] = "∫";
        map["oint"] = "∮";
        map["nabla"] = "∇";
        
        // Relations
        map["<="] = "≤";
        map[">="] = "≥";
        map["!="] = "≠";
        map["=="] = "≡";
        map["approx"] = "≈";
        map["prop"] = "∝";
        map["pm"] = "±";
        map["times"] = "×";
        map["div"] = "÷";
        map["cdot"] = "·";
        map["cdots"] = "⋯";
        map["ldots"] = "…";
        map["..."] = "…";
        
        // Sets
        map["!in"] = "∉";
        map["subset"] = "⊂";
        map["subseteq"] = "⊆";
        map["supset"] = "⊃";
        map["supseteq"] = "⊇";
        map["union"] = "∪";
        map["intersect"] = "∩";
        map["intersection"] = "∩";
        map["empty"] = "∅";
        map["emptyset"] = "∅";
        map["N"] = "ℕ";
        map["Z"] = "ℤ";
        map["Q"] = "ℚ";
        map["R"] = "ℝ";
        map["C"] = "ℂ";
        
        // Logic
        map["and"] = "∧";
        map["or"] = "∨";
        map["not"] = "¬";
        map["implies"] = "⇒";
        map["iff"] = "⇔";
        map["forall"] = "∀";
        map["exists"] = "∃";
        map["therefore"] = "∴";
        map["because"] = "∵";
        
        // Arrows
        map["->"] = "→";
        map["<-"] = "←";
        map["<->"] = "↔";
        map["=>"] = "⇒";
        map["<=="] = "⇐";
        map["<=>"] = "⇔";
        map["to"] = "→";
        map["gets"] = "←";
        
        // Geometry
        map["perp"] = "⊥";
        map["parallel"] = "∥";
        map["angle"] = "∠";
        map["triangle"] = "△";
        map["degree"] = "°";
        map["deg"] = "°";
        map["mid"] = "∣";
        
        // Misc
        map["infty"] = "∞";
        map["infinity"] = "∞";
        map["partial"] = "∂";
        map["hbar"] = "ℏ";
        map["ell"] = "ℓ";
        map["prime"] = "′";
        map["''"] = "″";
        map["'''"] = "‴";
        map["~"] = "≈";
        
        // Fractions (Unicode fractions)
        map["1/2"] = "½";
        map["1/3"] = "⅓";
        map["2/3"] = "⅔";
        map["1/4"] = "¼";
        map["3/4"] = "¾";
        map["1/5"] = "⅕";
        map["2/5"] = "⅖";
        map["3/5"] = "⅗";
        map["4/5"] = "⅘";
        map["1/6"] = "⅙";
        map["5/6"] = "⅚";
        map["1/8"] = "⅛";
        map["3/8"] = "⅜";
        map["5/8"] = "⅝";
        map["7/8"] = "⅞";
        map["1/9"] = "⅑";
        map["1/10"] = "⅒";
    }
    return map;
}

QString MathConverter::convert(const QString &input, int &charsToDelete)
{
    charsToDelete = 0;
    
    // Handle root(n) - nth root
    // Pattern: root(n) or root(n, x) -> ⁿ√x or √x
    if (input.startsWith("root(")) {
        int closeParen = input.indexOf(')');
        if (closeParen != -1) {
            QString inner = input.mid(5, closeParen - 5);
            QString after = input.mid(closeParen + 1);
            
            // Check if it's n,x format: root(3, 64) = cube root of 64
            if (inner.contains(',')) {
                QStringList parts = inner.split(',');
                if (parts.size() == 2) {
                    QString n = parts[0].trimmed();
                    QString x = parts[1].trimmed();
                    QString superscript = superscriptDigit(n);
                    if (!superscript.isEmpty()) {
                        charsToDelete = input.length();
                        return superscript + "√" + x + after;
                    }
                }
            }
            
            // root(n) format: root(3) = cube root
            QString n = inner.trimmed();
            if (!n.isEmpty() && n != "2") {
                QString superscript = superscriptDigit(n);
                if (!superscript.isEmpty()) {
                    charsToDelete = input.length();
                    return superscript + "√" + after;
                }
            }
            
            // Just root() = square root
            if (n.isEmpty() || n == "2") {
                charsToDelete = input.length();
                return "√" + after;
            }
        }
    }
    
    // Handle nroot(x) format: nroot(3, 64) = cube root of 64
    if (input.startsWith("nroot(")) {
        int closeParen = input.indexOf(')');
        if (closeParen != -1) {
            QString inner = input.mid(6, closeParen - 6);
            QString after = input.mid(closeParen + 1);
            
            if (inner.contains(',')) {
                QStringList parts = inner.split(',');
                if (parts.size() == 2) {
                    QString n = parts[0].trimmed();
                    QString x = parts[1].trimmed();
                    QString superscript = superscriptDigit(n);
                    if (!superscript.isEmpty()) {
                        charsToDelete = input.length();
                        return superscript + "√" + x + after;
                    }
                }
            }
        }
    }
    
    // Normal conversion from map
    static const QMap<QString, QString> &map = getMathMap();
    auto it = map.find(input);
    if (it != map.end()) {
        charsToDelete = input.length();
        return it.value();
    }
    
    return QString();
}
