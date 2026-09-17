# PicoQuant

### Bitcoin Options Pricing & Volatility Engine

An embedded quantitative finance project built on a **Raspberry Pi Pico W**.

PicoQuant combines live Bitcoin options data, historical BTC prices, volatility estimation, and Black–Scholes pricing.

## Features

- Raspberry Pi Pico W + Wi-Fi
- Live Bitcoin options data from **Deribit Testnet**
- Historical BTC/USD data from **Coinbase**
- Historical volatility calculation
- Implied volatility using **Newton–Raphson**
- Black–Scholes call pricing
- Market price vs model fair value comparison

## Methodology

### Historical Volatility

90 daily BTC/USD closing prices are used to calculate log returns and annualised historical volatility.

$$
r_t = \ln\left(\frac{P_t}{P_{t-1}}\right)
$$

$$
\sigma_{annual} = \sigma_{daily}\sqrt{365}
$$

### Implied Volatility

Newton–Raphson is used to find the volatility that makes the Black–Scholes price match the observed market price.

### Fair Volatility

Historical and implied volatility are currently combined using a 50/50 weighting:

$$
\sigma_{fair}
=
0.5\sigma_{historical}
+
0.5\sigma_{implied}
$$

The resulting volatility is used to calculate the model's fair option value.

## Hardware

- Raspberry Pi Pico W
- USB connection
- Optional I²C OLED display

## Data Sources

- **Deribit Testnet** — live Bitcoin option data
- **Coinbase Exchange API** — historical BTC/USD data

## Limitations

This is an educational quantitative finance prototype.

- 90-day historical volatility sample
- Simple 50/50 volatility weighting
- Black–Scholes assumes constant volatility
- Fixed risk-free rate
- No volatility surface modelling
- No automated trading

## Future Development

- EWMA volatility forecasting
- Volatility backtesting
- Realised vs forecast volatility
- Volatility clustering
- Volatility smile and skew analysis
- Alternative volatility models
- OLED display

## Why PicoQuant?

The project explores the implementation of quantitative finance models on a resource-constrained microcontroller rather than relying entirely on Python, R, or MATLAB.

This introduces practical challenges involving **memory, numerical precision, computation, networking, and JSON parsing**.

## Disclaimer

This project is for educational and research purposes only. Model outputs depend on the assumptions, numerical methods, and market data used.
