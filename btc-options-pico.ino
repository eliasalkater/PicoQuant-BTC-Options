#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


// ==================================================
// WI-FI
// ==================================================

const char* ssid = "Pixel";
const char* password = "YOUR_PASSWORD";


// ==================================================
// OPTION
// ==================================================

const char* optionInstrument = "BTC-25SEP26-75000-C";


// ==================================================
// HISTORICAL DATA SETTINGS
// ==================================================

const int numberOfItems = 90;
const int numberOfReturns = numberOfItems - 1;

float totalOfReturns = 0;
float totalSquareDeviations = 0;


// ==================================================
// NORMAL DISTRIBUTION CONSTANTS
// ==================================================

const float pi = 3.141592653589;
const float p = 0.2316419;

const float b1 = 0.319381530;
const float b2 = -0.356563782;
const float b3 = 1.781477937;
const float b4 = -1.821255978;
const float b5 = 1.330274429;


// ==================================================
// OPTION DATA STRUCT
// ==================================================

struct OptionData {

  float marketCallPrice;
  float currentPrice;
  float strikePrice = 75000;
  float timeToExpiry;
  float riskFreeRate = 0.04;

};

OptionData option;


// ==================================================
// NORMAL DISTRIBUTION
// ==================================================

float phi(float x) {

  float result =
    exp((-x * x) / 2) /
    sqrt(2 * pi);

  return result;
}


float t(float x) {

  float result =
    1 / (1 + p * x);

  return result;
}


float normalCDF(float x) {

  float positiveX = abs(x);

  float result =
    1 -
    phi(positiveX) *
    (
      (b1 * pow(t(positiveX), 1)) +
      (b2 * pow(t(positiveX), 2)) +
      (b3 * pow(t(positiveX), 3)) +
      (b4 * pow(t(positiveX), 4)) +
      (b5 * pow(t(positiveX), 5))
    );

  if (x >= 0) {
    return result;
  }

  else {
    return 1 - result;
  }
}


// ==================================================
// BLACK-SCHOLES CALL PRICE
// ==================================================

float blackScholesCall(float volatility) {

  float logPriceRatio =
    log(option.currentPrice / option.strikePrice);

  float sigmaSquaredHalf =
    (volatility * volatility) / 2;

  float d1 =
    (
      logPriceRatio +
      (
        (option.riskFreeRate + sigmaSquaredHalf)
        * option.timeToExpiry
      )
    )
    /
    (
      volatility *
      sqrt(option.timeToExpiry)
    );

  float d2 =
    d1 -
    (
      volatility *
      sqrt(option.timeToExpiry)
    );

  float Nd1 =
    normalCDF(d1);

  float Nd2 =
    normalCDF(d2);

  float presentStrike =
    option.strikePrice *
    exp(
      -option.riskFreeRate *
      option.timeToExpiry
    );

  float callPrice =
    (option.currentPrice * Nd1) -
    (presentStrike * Nd2);

  return callPrice;
}


// ==================================================
// BLACK-SCHOLES VEGA
// ==================================================

float blackScholesVega(float volatility) {

  float logPriceRatio =
    log(option.currentPrice / option.strikePrice);

  float sigmaSquaredHalf =
    (volatility * volatility) / 2;

  float d1 =
    (
      logPriceRatio +
      (
        (option.riskFreeRate + sigmaSquaredHalf)
        * option.timeToExpiry
      )
    )
    /
    (
      volatility *
      sqrt(option.timeToExpiry)
    );

  float vega =
    option.currentPrice *
    phi(d1) *
    sqrt(option.timeToExpiry);

  return vega;
}


// ==================================================
// IMPLIED VOLATILITY
// ==================================================

float impliedVolatility(float marketCallPrice) {

  float volatilityGuess = 0.5;

  float acceptableError = 0.1;

  float actualError =
    abs(
      blackScholesCall(volatilityGuess)
      - marketCallPrice
    );

  int iterations = 0;


  while (actualError > acceptableError) {

    float vega =
      blackScholesVega(volatilityGuess);

    // Prevent division by zero
    if (vega < 0.000001) {
      break;
    }

    volatilityGuess =
      volatilityGuess -
      (
        (
          blackScholesCall(volatilityGuess)
          - marketCallPrice
        )
        /
        vega
      );


    // Prevent an invalid negative volatility
    if (volatilityGuess <= 0) {
      volatilityGuess = 0.01;
    }


    actualError =
      abs(
        blackScholesCall(volatilityGuess)
        - marketCallPrice
      );


    iterations++;


    if (iterations > 100) {
      break;
    }
  }


  return volatilityGuess;
}


// ==================================================
// GET LIVE OPTION DATA FROM DERIBIT
// ==================================================

float getMarketCallPrice() {

  HTTPClient http;

  http.setInsecure();


  String orderBookURL =
    "https://test.deribit.com/api/v2/public/get_order_book?instrument_name="
    + String(optionInstrument);


  http.begin(orderBookURL);

  int httpResponseCode =
    http.GET();


  if (httpResponseCode != 200) {

    http.end();

    return -1;
  }


  String httpText =
    http.getString();


  JsonDocument doc;

  DeserializationError error =
    deserializeJson(doc, httpText);


  if (error) {

    http.end();

    return -1;
  }


  float bestBid =
    doc["result"]["best_bid_price"];

  float bestAsk =
    doc["result"]["best_ask_price"];

  float underlyingPrice =
    doc["result"]["underlying_price"];


  option.currentPrice =
    underlyingPrice;


  long long currentTimestamp =
    doc["result"]["timestamp"];


  http.end();


  // --------------------------------------------------
  // GET OPTION EXPIRATION
  // --------------------------------------------------

  HTTPClient instrumentHttp;

  instrumentHttp.setInsecure();


  String instrumentURL =
    "https://test.deribit.com/api/v2/public/get_instrument?instrument_name="
    + String(optionInstrument);


  instrumentHttp.begin(instrumentURL);


  int instrumentResponseCode =
    instrumentHttp.GET();


  if (instrumentResponseCode != 200) {

    instrumentHttp.end();

    return -1;
  }


  String instrumentText =
    instrumentHttp.getString();


  JsonDocument instrumentDoc;

  DeserializationError instrumentError =
    deserializeJson(
      instrumentDoc,
      instrumentText
    );


  if (instrumentError) {

    instrumentHttp.end();

    return -1;
  }


  long long expirationTimestamp =
    instrumentDoc["result"]["expiration_timestamp"];


  option.timeToExpiry =
    (
      expirationTimestamp -
      currentTimestamp
    )
    /
    (1000.0 * 60 * 60 * 24 * 365);


  instrumentHttp.end();


  // --------------------------------------------------
  // OPTION MIDPOINT
  // --------------------------------------------------

  float midpointBTC =
    (bestBid + bestAsk) / 2;


  float midpointUSD =
    midpointBTC *
    underlyingPrice;


  return midpointUSD;
}


// ==================================================
// GET HISTORICAL BTC PRICES
// ==================================================

bool getHistoricalBTCPrices(float prices[]) {

  HTTPClient http;

  http.setInsecure();


  http.begin(
    "https://api.exchange.coinbase.com/products/BTC-USD/candles?granularity=86400"
  );


  int httpResponseCode =
    http.GET();


  if (httpResponseCode != 200) {

    http.end();

    return false;
  }


  String httpText =
    http.getString();


  JsonDocument doc;

  DeserializationError error =
    deserializeJson(doc, httpText);


  if (error) {

    http.end();

    return false;
  }


  int numberOfCandles =
    doc.size();


  if (numberOfCandles < numberOfItems) {

    http.end();

    return false;
  }


  // Coinbase returns newest first.
  // Reverse the order so prices[0] is oldest.

  for (
    int i = 0;
    i < numberOfItems;
    i++
  ) {

    int sourceIndex =
      numberOfItems - 1 - i;


    prices[i] =
      doc[sourceIndex][4];
  }


  http.end();

  return true;
}


// ==================================================
// CLEAN SERIAL SUMMARY
// ==================================================

void printCleanSummary(
  float historicalVolatility,
  float impliedVol,
  float fairVolatility,
  float fairCallPrice,
  float mispricingPercentage
) {

  Serial.println();

  Serial.println(
    "========================================"
  );

  Serial.println(
    "       BTC OPTION PRICING MODEL"
  );

  Serial.println(
    "========================================"
  );


  Serial.print(
    "Instrument            "
  );

  Serial.println(
    optionInstrument
  );


  Serial.print(
    "BTC Price             $"
  );

  Serial.println(
    option.currentPrice,
    2
  );


  Serial.print(
    "Strike                $"
  );

  Serial.println(
    option.strikePrice,
    2
  );


  Serial.print(
    "Market Option Price   $"
  );

  Serial.println(
    option.marketCallPrice,
    2
  );


  Serial.print(
    "Time to Expiry        "
  );

  Serial.print(
    option.timeToExpiry * 365,
    1
  );

  Serial.println(
    " days"
  );


  Serial.println();


  Serial.print(
    "Historical Volatility "
  );

  Serial.print(
    historicalVolatility * 100,
    2
  );

  Serial.println("%");


  Serial.print(
    "Implied Volatility     "
  );

  Serial.print(
    impliedVol * 100,
    2
  );

  Serial.println("%");


  Serial.print(
    "Fair Volatility        "
  );

  Serial.print(
    fairVolatility * 100,
    2
  );

  Serial.println("%");


  Serial.println();


  Serial.print(
    "Model Fair Value      $"
  );

  Serial.println(
    fairCallPrice,
    2
  );


  Serial.print(
    "Market vs Model       "
  );

  Serial.print(
    mispricingPercentage * 100,
    2
  );

  Serial.println("%");


  Serial.println();


  Serial.print(
    "Risk-Free Rate        "
  );

  Serial.print(
    option.riskFreeRate * 100,
    2
  );

  Serial.println("%");


  Serial.println(
    "========================================"
  );
}


// ==================================================
// SETUP
// ==================================================

void setup() {

  Serial.begin(115200);

  delay(1000);


  // ==================================================
  // CONNECT TO WI-FI
  // ==================================================

  WiFi.begin(
    ssid,
    password
  );


  while (
    WiFi.status() != WL_CONNECTED
  ) {

    delay(500);
  }


  // ==================================================
  // GET LIVE OPTION PRICE
  // ==================================================

  option.marketCallPrice =
    getMarketCallPrice();


  if (
    option.marketCallPrice <= 0
  ) {

    Serial.println(
      "ERROR: Could not get market option price."
    );

    return;
  }


  // ==================================================
  // GET HISTORICAL BTC DATA
  // ==================================================

  float btc[numberOfItems];


  bool historicalDataSuccess =
    getHistoricalBTCPrices(btc);


  if (!historicalDataSuccess) {

    Serial.println(
      "ERROR: Could not get historical BTC prices."
    );

    return;
  }


  // ==================================================
  // CALCULATE LOG RETURNS
  // ==================================================

  float logReturns[numberOfReturns];


  totalOfReturns = 0;


  for (
    int i = 0;
    i < numberOfReturns;
    i++
  ) {

    float returnValue =
      log(
        btc[i + 1] /
        btc[i]
      );


    logReturns[i] =
      returnValue;


    totalOfReturns +=
      returnValue;
  }


  // ==================================================
  // MEAN RETURN
  // ==================================================

  float meanReturn =
    totalOfReturns /
    numberOfReturns;


  // ==================================================
  // VARIANCE
  // ==================================================

  totalSquareDeviations = 0;


  for (
    int i = 0;
    i < numberOfReturns;
    i++
  ) {

    float deviation =
      logReturns[i] -
      meanReturn;


    float squaredDeviation =
      deviation *
      deviation;


    totalSquareDeviations +=
      squaredDeviation;
  }


  float variance =
    totalSquareDeviations /
    (numberOfReturns - 1);


  float standardDeviation =
    sqrt(variance);


  // ==================================================
  // HISTORICAL VOLATILITY
  // ==================================================

  float historicalVolatility =
    sqrt(365) *
    standardDeviation;


  // ==================================================
  // IMPLIED VOLATILITY
  // ==================================================

  float impliedVol =
    impliedVolatility(
      option.marketCallPrice
    );


  // ==================================================
  // FAIR VOLATILITY
  // ==================================================

  float k = 0.5;


  float fairVolatility =
    (k * historicalVolatility) +
    ((1 - k) * impliedVol);


  // ==================================================
  // FAIR OPTION PRICE
  // ==================================================

  float fairCallPrice =
    blackScholesCall(
      fairVolatility
    );


  // ==================================================
  // MODEL DIFFERENCE
  // ==================================================

  float mispricingPercentage =
    (
      option.marketCallPrice -
      fairCallPrice
    )
    /
    option.marketCallPrice;


  // ==================================================
  // CLEAN OUTPUT
  // ==================================================

  printCleanSummary(
    historicalVolatility,
    impliedVol,
    fairVolatility,
    fairCallPrice,
    mispricingPercentage
  );
}


// ==================================================
// LOOP
// ==================================================

void loop() {

}
