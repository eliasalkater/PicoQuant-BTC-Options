# PicoQuant

### Bitcoin Options Pricing & Volatility Engine

An embedded quantitative finance project built on a **Raspberry Pi Pico W**.

PicoQuant combines live Bitcoin options data, historical BTC prices, volatility estimation, and Black–Scholes option pricing into a single embedded system.

## Features

- Raspberry Pi Pico W + Wi-Fi
- Live Bitcoin option data from **Deribit Testnet**
- Historical BTC/USD data from **Coinbase**
- Log-return calculations
- Annualised historical volatility
- Implied volatility using **Newton–Raphson**
- Black–Scholes call pricing
- Historical + implied volatility blending
- Market price vs model fair value comparison

## How It Works

### 1. Historical Volatility

The system retrieves 90 daily BTC/USD closing prices and calculates logarithmic returns:

$$
r_t = \ln\left(\frac{P_t}{P_{t-1}}\right)
$$

Daily volatility is then annualised using:

$$
\sigma_{annual} = \sigma_{daily}\sqrt{365}
$$

### 2. Black–Scholes

The project calculates the theoretical price of a European call option using:

- BTC price
- Strike price
- Time to expiry
- Risk-free rate
- Volatility

$$
C = S N(d_1) - K e^{-rT}N(d_2)
$$

### 3. Implied Volatility

Newton–Raphson is used to find the volatility that makes the Black–Scholes price match the observed market price.

$$
\sigma_{n+1}
=
\sigma_n -
\frac{C(\sigma_n)-C_{market}}{Vega(\sigma_n)}
$$

### 4. Fair Volatility

The current prototype combines historical and implied volatility:

$$
\sigma_{fair}
=
0.5\sigma_{historical}
+
0.5\sigma_{implied}
$$

The resulting volatility is used to calculate the model's fair option value.

## Example

Example output from the Pico W:

```text
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
```
