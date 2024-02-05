#include <time.h>

// JJY処理
#pragma region JJY

#define JJY_MARKER 0x02
#define JJY_ON 0x05
#define JJY_OFF 0x08
#define JJY_IS(c) c ? JJY_ON : JJY_OFF
#define JJY_GET_ONTIME(v) v * 100

/*
 最大2桁の10進数をBCDに変換
 c: 変換する10進数の値
 */
unsigned char bin2bcd2(unsigned int c) {
  // 3シフト目までは必ず条件に一致しないので飛ばす
  c = c << 3;
  for (int i = 3; i < 8; ++i) {
    if ((c & 0xF000) > 0x4000)
      c = c + 0x3000;
    if ((c & 0xF00) > 0x400)
      c = c + 0x300;
    c = c << 1;
  }
  return c >> 8;
}

/*
 最大3桁の10進数をBCDに変換
  c: 変換する10進数の値
 */
unsigned short bin2bcd3(unsigned int c) {
  // 3シフト目までは必ず条件に一致しないので飛ばす
  c = c << 3;
  for (int i = 3; i < 12; ++i) {
    if ((c & 0xF00000) > 0x400000)
      c = c + 0x300000;
    if ((c & 0xF0000) > 0x40000)
      c = c + 0x30000;
    if ((c & 0xF000) > 0x4000)
      c = c + 0x3000;
    c = c << 1;
  }
  return c >> 12;
}

/*
 8bit分の偶数パリティを計算
  c: 計算対象の値
 */
inline unsigned char calcParity(unsigned char c) {
  // NOTE: https://qiita.com/shiozaki/items/e803483cbd8b3c6ab28d
  c ^= c >> 4;
  c ^= c >> 2;
  c ^= c >> 1;
  return c & 0x01;
}

/*
 指定した日時のJJYフォーマットのタイムコードを生成
 time: タイムコードを生成する対象の時間
 pTimecode: 生成したタイムコードを格納する60バイトの配列
 */
void createJjyTimeCode(const struct tm* time, unsigned char pTimecode[60]) {
  unsigned int tmp;

  // NOTE: https://www.nict.go.jp/sts/jjy_signal.html
  // NOTE: 毎時15分および45分の呼出符号および停波予告は、一般的な電波時計には無関係と思われるので対応しない

  pTimecode[0] = JJY_MARKER;

  // 分 (1 - 8秒)
  tmp = bin2bcd2(time->tm_min);
  pTimecode[1] = JJY_IS(tmp & 0x40);
  pTimecode[2] = JJY_IS(tmp & 0x20);
  pTimecode[3] = JJY_IS(tmp & 0x10);
  pTimecode[4] = JJY_OFF;
  pTimecode[5] = JJY_IS(tmp & 0x08);
  pTimecode[6] = JJY_IS(tmp & 0x04);
  pTimecode[7] = JJY_IS(tmp & 0x02);
  pTimecode[8] = JJY_IS(tmp & 0x01);
  // パリティ:分 (37秒)
  pTimecode[37] = JJY_IS(calcParity(tmp));

  pTimecode[9] = JJY_MARKER;

  // 時 (10 - 18秒)
  tmp = bin2bcd2(time->tm_hour);
  pTimecode[10] = JJY_OFF;
  pTimecode[11] = JJY_OFF;
  pTimecode[12] = JJY_IS(tmp & 0x20);
  pTimecode[13] = JJY_IS(tmp & 0x10);
  pTimecode[14] = JJY_OFF;
  pTimecode[15] = JJY_IS(tmp & 0x08);
  pTimecode[16] = JJY_IS(tmp & 0x04);
  pTimecode[17] = JJY_IS(tmp & 0x02);
  pTimecode[18] = JJY_IS(tmp & 0x01);
  // パリティ:時 (36秒)
  pTimecode[36] = JJY_IS(calcParity(tmp));

  pTimecode[19] = JJY_MARKER;

  // 0固定 (20, 21秒)
  pTimecode[20] = JJY_OFF;
  pTimecode[21] = JJY_OFF;

  // 通算日 (22 - 33秒)
  tmp = bin2bcd3(time->tm_yday + 1);
  pTimecode[22] = JJY_IS(tmp & 0x200);
  pTimecode[23] = JJY_IS(tmp & 0x100);
  pTimecode[24] = JJY_OFF;
  pTimecode[25] = JJY_IS(tmp & 0x80);
  pTimecode[26] = JJY_IS(tmp & 0x40);
  pTimecode[27] = JJY_IS(tmp & 0x20);
  pTimecode[28] = JJY_IS(tmp & 0x10);
  pTimecode[29] = JJY_MARKER;
  pTimecode[30] = JJY_IS(tmp & 0x8);
  pTimecode[31] = JJY_IS(tmp & 0x4);
  pTimecode[32] = JJY_IS(tmp & 0x2);
  pTimecode[33] = JJY_IS(tmp & 0x1);

  // 0固定 (34, 35秒)
  pTimecode[34] = JJY_OFF;
  pTimecode[35] = JJY_OFF;

  // 予備 (38秒)
  pTimecode[38] = JJY_OFF;
  pTimecode[39] = JJY_MARKER;

  // 予備 (40秒)
  pTimecode[40] = JJY_OFF;

  // 西暦年の下２桁 (41 - 48秒)
  // FIXME: 2100年前後に使う場合は要修正(その時代を生きる人にお任せします。)
  tmp = bin2bcd2(time->tm_year - 100);
  pTimecode[41] = JJY_IS(tmp & 0x80);
  pTimecode[42] = JJY_IS(tmp & 0x40);
  pTimecode[43] = JJY_IS(tmp & 0x20);
  pTimecode[44] = JJY_IS(tmp & 0x10);
  pTimecode[45] = JJY_IS(tmp & 0x8);
  pTimecode[46] = JJY_IS(tmp & 0x4);
  pTimecode[47] = JJY_IS(tmp & 0x2);
  pTimecode[48] = JJY_IS(tmp & 0x1);

  pTimecode[49] = JJY_MARKER;

  // 曜日 (50 - 52秒)
  // NOTE: 1桁はBCDに変換しても変わらないのでそのまま使用
  pTimecode[50] = JJY_IS(time->tm_wday & 0x4);
  pTimecode[51] = JJY_IS(time->tm_wday & 0x2);
  pTimecode[52] = JJY_IS(time->tm_wday & 0x1);

  // うるう秒の予告 (53 - 54秒)
  // NOTE: うるう秒でズレても次の同期で合うので考慮しない
  pTimecode[53] = JJY_OFF;
  pTimecode[54] = JJY_OFF;

  // 0固定 (55 - 58秒)
  pTimecode[55] = JJY_OFF;
  pTimecode[56] = JJY_OFF;
  pTimecode[57] = JJY_OFF;
  pTimecode[58] = JJY_OFF;

  pTimecode[59] = JJY_MARKER;
}

#pragma endregion

void setup() {
  // デバッグ出力用のシリアル出力を初期化
  Serial.begin(115200);

  pinMode(32, OUTPUT);

  struct tm tm_now;
  tm_now.tm_year = 2034 - 1900;
  tm_now.tm_mon = 6;
  tm_now.tm_mday = 14;
  tm_now.tm_hour = 11;
  tm_now.tm_min = 34;
  tm_now.tm_sec = 0;
  tm_now.tm_wday = 5;
  tm_now.tm_yday = 194;
  tm_now.tm_isdst = 0;

  struct timeval tv = { mktime(&tm_now), 0 };
  settimeofday(&tv, NULL);
  
  struct timespec nowTime;
  if (clock_gettime(CLOCK_REALTIME, &nowTime) == -1) {
    // TODO: ERROR
  }
  struct tm* nowDateTime = localtime(&nowTime.tv_sec);

  log_d("settimeofday(%d/%d/%d %d:%d:%d)",
        nowDateTime->tm_year + 1900, nowDateTime->tm_mon + 1, nowDateTime->tm_mday,
        nowDateTime->tm_hour, nowDateTime->tm_min, nowDateTime->tm_sec);

  log_d("Setup succeeded!");
}

void loop() {
  struct timespec nowTime;
  if (clock_gettime(CLOCK_REALTIME, &nowTime) == -1) {
    // TODO: ERROR
  }
  struct tm* nowDateTime = localtime(&nowTime.tv_sec);

  unsigned char timecode[60];

  createJjyTimeCode(nowDateTime, timecode);

  for (int i = nowDateTime->tm_sec; i < 60; ++i) {
    int ontime = JJY_GET_ONTIME(timecode[i]);
    digitalWrite(32, HIGH);
    delay(ontime);
    digitalWrite(32, LOW);
    delay(1000 - ontime);
  }
}