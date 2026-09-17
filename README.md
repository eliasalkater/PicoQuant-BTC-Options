PicoQuant — Bitcoin Options Pricing & Volatility Engine

An embedded quantitative finance project built on a Raspberry Pi Pico W that combines live Bitcoin options data, historical Bitcoin prices, volatility estimation, and Black–Scholes option pricing.

The project explores how quantitative finance models can be implemented on a resource-constrained microcontroller while interacting with real market data APIs.

Features
Raspberry Pi Pico W implementation
Wi-Fi connectivity
Live Bitcoin option data from Deribit Testnet
Historical BTC/USD data from the Coinbase Exchange API
Log-return calculation
Sample variance calculation
Annualised historical volatility
Implied volatility estimation using Newton–Raphson
Black–Scholes European call option pricing
Historical and implied volatility blending
Market price vs model fair-value comparison
Clean quantitative output through the Serial Monitor
Quantitative Methodology
Historical Volatility

The project retrieves 90 daily BTC/USD closing prices and calculates logarithmic returns:

rₜ = ln(Pₜ / Pₜ₋₁)

The sample variance of the returns is calculated and converted into annualised volatility:

σannual = σdaily × √365

Bitcoin trades continuously, so 365 days are currently used for annualisation.

Black–Scholes

The project implements the Black–Scholes model for European call options.

The model uses:

Current BTC price
Strike price
Time to expiry
Risk-free rate
Volatility

The call price is calculated using:

C = S N(d₁) - K e^(-rT) N(d₂)

where:

d₁ = [ln(S/K) + (r + σ²/2)T] / (σ√T)

d₂ = d₁ - σ√T
Implied Volatility

The market option price is used to solve for the volatility that causes the Black–Scholes model to reproduce the observed market price.

The project uses the Newton–Raphson method:

σₙ₊₁ = σₙ - [C(σₙ) - Cmarket] / Vega(σₙ)

This allows the model to extract implied volatility from the observed option price.

Fair Volatility

The current prototype combines historical and implied volatility using an equal weighting:

σfair = 0.5σhistorical + 0.5σimplied

This is a simple modelling assumption and is not intended to represent a calibrated market model.

Model vs Market

The resulting fair volatility is used as the volatility input for the Black–Scholes model to calculate a model fair value.

The project then calculates the percentage difference between the observed market midpoint and the model value:

Difference = (Market Price - Model Price) / Market Price

This represents a model-relative pricing difference and should not be interpreted as proof that an option is objectively mispriced.

Example Output

An example run produces output similar to:

========================================
       BTC OPTION PRICING MODEL
========================================
Instrument            BTC-25SEP26-75000-C
BTC Price             $75880.95
Strike                $75000.00
Market Option Price   $2029.82
Time to Expiry        8.4 days

Historical Volatility 36.73%
Implied Volatility     33.00%
Fair Volatility        34.87%

Model Fair Value      $2112.38
Market vs Model       -4.07%

Risk-Free Rate        4.00%
========================================

Values change between runs because the project uses live market data.

Hardware
Raspberry Pi Pico W
Computer for programming and monitoring
Optional I²C OLED display for future development

The OLED display is currently not part of the core implementation.

Data Sources
Deribit Testnet

Used to retrieve live Bitcoin option order-book data, underlying BTC price, and option instrument information.

Coinbase Exchange API

Used to retrieve historical BTC/USD daily candle data for volatility calculations.

Limitations

This is a quantitative finance prototype rather than a production trading or valuation system.

Current limitations include:

Historical volatility uses a relatively small 90-day sample.
The volatility model uses a simple 50/50 historical/implied weighting.
Black–Scholes assumes constant volatility.
The model does not currently incorporate the Bitcoin options volatility surface.
Bid/ask spreads and market liquidity are not explicitly modelled.
The risk-free rate is currently a fixed modelling assumption.
Deribit Testnet data is used rather than production execution.
No trading decisions or automated orders are executed.
Future Development

Planned extensions include:

EWMA volatility forecasting
Volatility forecasting evaluation
Backtesting against realised volatility
Larger historical datasets
Volatility clustering analysis
Volatility smile and skew analysis
Comparison of alternative volatility models
Improved calibration of the fair-volatility model
OLED-based embedded display
Performance and memory optimisation for the Pico W
Why a Pico W?

Quantitative finance projects are commonly developed using environments such as Python, R, or MATLAB.

This project deliberately implements the core calculations on a microcontroller, introducing constraints around:

Memory
Computational resources
Numerical precision
Networking
JSON parsing
Embedded implementation

The aim is to explore how financial mathematics can be translated into a constrained embedded environment rather than relying entirely on high-level quantitative libraries.

Disclaimer

This project is for educational and research purposes.

Calculated model values depend on the assumptions, numerical methods, and market data used by the implementation. They should not be interpreted as financial advice or guaranteed market valuations.
