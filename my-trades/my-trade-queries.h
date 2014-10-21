/* my-trade-queries.h
 * part of my-trades
 */

  char	*query_list="select distinct(symbol) from stockinfo \
    where exchange in (\"NasdaqNM\",\"NGM\", \"NCM\", \"NYSE\") \
    and active = true \
    and p_e_ratio is not null \
    and capitalisation is not null \
    and low_52weeks > "MINPRICE" \
    and high_52weeks < "MAXPRICE" \
    and avg_volume > "MINVOLUME" \
    order by symbol";
  char	*create_table="create temporary table if not exists TRADES ( \
    SYMBOL VARCHAR(10) UNIQUE, \
    OB BIT(1) NOT NULL DEFAULT false, \
    KR BIT(1) NOT NULL DEFAULT false, \
    TREND BIT(1) NOT NULL DEFAULT false, \
    MACD BIT(1) NOT NULL DEFAULT false, \
    HV BIT(1) NOT NULL DEFAULT false, \
    PP DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    R1 DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    R2 DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    S1 DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    S2 DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    CHANUP DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    CHANDN DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    SAFEUP DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    SAFEDN DECIMAL(12,4) UNSIGNED NULL DEFAULT NULL, \
    BID DECIMAL(12,4) UNSIGNED NOT NULL DEFAULT 0.0, \
    ASK DECIMAL(12,4) UNSIGNED NOT NULL DEFAULT 0.0)";
