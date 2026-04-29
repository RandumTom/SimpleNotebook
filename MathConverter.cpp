#include "MathConverter.h"
#include <QString>
#include <QMap>
#include <cmath>

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

static bool superscriptToInt(const QChar &c, int &n)
{
    static const QMap<QChar, int> map = {
        {QChar(8304), 0},
        {QChar(185), 1},
        {QChar(178), 2},
        {QChar(179), 3},
        {QChar(8308), 4},
        {QChar(8309), 5},
        {QChar(8310), 6},
        {QChar(8311), 7},
        {QChar(8312), 8},
        {QChar(8313), 9},
        {QChar(8317), -1}
    };

    auto it = map.find(c);
    if (it != map.end()) {
        n = it.value();
        return true;
    }
    return false;
}

static double parseUnicodeRoot(const QString &expr, int &pos)
{
    if (pos >= expr.length()) return NAN;

    QChar c = expr[pos];
    int numStart;

    if (c == QChar(0x221A)) {
        pos++;
        numStart = pos;
        while (pos < expr.length() && (expr[pos].isDigit() || expr[pos] == '.')) {
            pos++;
        }
        QString numStr = expr.mid(numStart, pos - numStart);
        if (numStr.isEmpty()) return NAN;
        return sqrt(numStr.toDouble());
    }

    int n = 0;
    if (superscriptToInt(c, n)) {
        if (n < 0) return NAN;
        pos++;
        if (pos < expr.length() && expr[pos] == QChar(0x221A)) {
            pos++;
            numStart = pos;
            while (pos < expr.length() && (expr[pos].isDigit() || expr[pos] == '.')) {
                pos++;
            }
            QString numStr = expr.mid(numStart, pos - numStart);
            if (numStr.isEmpty()) return NAN;
            return pow(numStr.toDouble(), 1.0 / n);
        }
    }

    return NAN;
}

// Helper to apply an operator to two numbers
static double applyOp(double a, double b, const QString &op)
{
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") return (b != 0) ? a / b : 0;
    if (op == "^") return pow(a, b);
    return b;
}

// Map of LaTeX input -> Unicode output (exact matches)
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
        
        // Superscripts (standalone - when user types just ^2)
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
        
        // Subscripts (standalone)
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
    
    if (input.isEmpty())
        return QString();
    
    // First check exact matches in map
    static const QMap<QString, QString> &map = getMathMap();
    auto it = map.find(input);
    if (it != map.end()) {
        charsToDelete = input.length();
        return it.value();
    }
    
    // Handle (x)root(y) format = y-th root of x
    // Example: (8)root(9) = ⁹√8
    if (input.contains(")root(")) {
        int rootPos = input.indexOf(")root(");
        QString x = input.left(rootPos); // Content before )root(
        QString afterRoot = input.mid(rootPos + 5); // After )root(
        int closeParen = afterRoot.lastIndexOf(')');
        if (closeParen > 0) {
            QString y = afterRoot.left(closeParen);
            QString after = afterRoot.mid(closeParen + 1);
            
            QString superscript = superscriptDigit(y.trimmed());
            if (!superscript.isEmpty()) {
                charsToDelete = input.length();
                return superscript + "√" + x + after;
            }
        }
    }
    
    // Handle root(n) format - square root of n (or n-th root if specified)
    // Examples: root(9) -> √9, root(3,64) -> ³√64
    if (input.startsWith("root(")) {
        int closeParen = input.lastIndexOf(')');
        if (closeParen > 5) {
            QString inner = input.mid(5, closeParen - 5);
            QString after = input.mid(closeParen + 1);
            
            // Check for n,x format: root(3,64) = cube root of 64
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
            
            // Just root(n) = square root of n
            QString n = inner.trimmed();
            if (!n.isEmpty()) {
                charsToDelete = input.length();
                return "√" + n + after;
            }
        }
    }
    
    return QString();
}

QString MathConverter::evaluate(const QString &input, int &charsToDelete)
{
    // Handle expressions ending with = (e.g. "5+3=" → "8")
    if (!input.endsWith('=')) {
        charsToDelete = 0;
        return QString();
    }
    
    QString expr = input.left(input.length() - 1).trimmed();
    if (expr.isEmpty()) {
        charsToDelete = 0;
        return QString();
    }
    
    // Simple expression parser for +, -, *, /, ^
    double result = 0;
    QString currentNum;
    QString lastOp = "+";
    bool hasOperator = false;
    
    for (int i = 0; i < expr.length(); i++) {
        QChar c = expr[i];

        int savedPos = i;
        double rootValue = parseUnicodeRoot(expr, i);
        if (!std::isnan(rootValue)) {
            if (!currentNum.isEmpty()) {
                double num = currentNum.toDouble();
                result = applyOp(result, num, lastOp);
                currentNum.clear();
            }
            result = applyOp(result, rootValue, lastOp);
            hasOperator = true;
            i = i - 1;
            continue;
        }
        i = savedPos;

        if (c.isDigit() || c == '.' || (c == '-' && currentNum.isEmpty() && !hasOperator)) {
            currentNum += c;
        } else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
            if (!currentNum.isEmpty()) {
                double num = currentNum.toDouble();
                result = applyOp(result, num, lastOp);
                currentNum.clear();
            }
            lastOp = c;
            hasOperator = true;
        }
    }
    
    // Process last number
    if (!currentNum.isEmpty()) {
        double num = currentNum.toDouble();
        result = applyOp(result, num, lastOp);
    }
    
    // Format result
    QString resultStr;
    if (result == static_cast<long long>(result)) {
        resultStr = QString::number(static_cast<long long>(result));
    } else {
        resultStr = QString::number(result, 'g', 10);
        // Remove trailing zeros
        if (resultStr.contains('.') && !resultStr.contains('e')) {
            while (resultStr.endsWith('0')) {
                resultStr.chop(1);
            }
            if (resultStr.endsWith('.')) {
                resultStr.chop(1);
            }
        }
    }
    
    charsToDelete = input.length();
    return resultStr;
}
