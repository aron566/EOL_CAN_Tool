/**
 *  @file utility.cpp
 *
 *  @date 2023年02月21日 17:58:46 星期二
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2022 aron566 <aron566@163.com>.
 *
 *  @brief None.
 *
 *  @details None.
 *
 *  @version v1.0.0 aron566 2023.02.21 17:58 初始版本.
 */
/** Includes -----------------------------------------------------------------*/
#include <QTextCodec>
#include <QByteArray>
#include <QFile>
#include <QTextStream>
/** Private includes ---------------------------------------------------------*/
#include "utility.h"
#include "qdebug.h"
/** Use C compiler -----------------------------------------------------------*/

/** Private macros -----------------------------------------------------------*/

/** Private typedef ----------------------------------------------------------*/

/** Private constants --------------------------------------------------------*/
static const uint8_t auchCRCHi[] = {
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
  0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
  0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
  0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
  0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
  0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
  0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
  0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
  0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
  0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

static const uint8_t auchCRCLo[] = {
  0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
  0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
  0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
  0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
  0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
  0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
  0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
  0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
  0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
  0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
  0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
  0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
  0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
  0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
  0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
  0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
  0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
  0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
  0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
  0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
  0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
  0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
  0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
  0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
  0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
  0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

/* crc32表 */
#define POLYNOMIAL    0x04c11db7L

static const uint32_t crc32_table[] = {
  0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
  0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
  0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
  0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
  0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
  0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
  0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
  0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
  0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
  0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
  0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
  0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
  0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
  0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
  0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
  0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
  0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
  0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
  0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
  0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
  0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
  0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
  0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
  0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
  0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
  0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
  0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
  0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
  0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
  0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
  0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
  0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
  0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
  0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
  0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
  0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
  0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
  0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
  0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
  0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
  0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
  0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
  0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
  0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
  0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
  0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
  0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
  0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
  0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
  0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
  0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
  0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
  0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
  0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
  0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
  0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
  0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
  0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
};

/* #define POLYNOMIAL 0xD419CC15L */
static const uint32_t crc32_table1[256] = {
  0x00000000, 0xD419CC15, 0x7C2A543F, 0xA833982A, 0xF854A87E, 0x2C4D646B, 0x847EFC41, 0x50673054, 0x24B09CE9,
  0xF0A950FC, 0x589AC8D6, 0x8C8304C3, 0xDCE43497, 0x08FDF882, 0xA0CE60A8, 0x74D7ACBD, 0x496139D2, 0x9D78F5C7,
  0x354B6DED, 0xE152A1F8, 0xB13591AC, 0x652C5DB9, 0xCD1FC593, 0x19060986, 0x6DD1A53B, 0xB9C8692E, 0x11FBF104,
  0xC5E23D11, 0x95850D45, 0x419CC150, 0xE9AF597A, 0x3DB6956F, 0x92C273A4, 0x46DBBFB1, 0xEEE8279B, 0x3AF1EB8E,
  0x6A96DBDA, 0xBE8F17CF, 0x16BC8FE5, 0xC2A543F0, 0xB672EF4D, 0x626B2358, 0xCA58BB72, 0x1E417767, 0x4E264733,
  0x9A3F8B26, 0x320C130C, 0xE615DF19, 0xDBA34A76, 0x0FBA8663, 0xA7891E49, 0x7390D25C, 0x23F7E208, 0xF7EE2E1D,
  0x5FDDB637, 0x8BC47A22, 0xFF13D69F, 0x2B0A1A8A, 0x833982A0, 0x57204EB5, 0x07477EE1, 0xD35EB2F4, 0x7B6D2ADE,
  0xAF74E6CB, 0xF19D2B5D, 0x2584E748, 0x8DB77F62, 0x59AEB377, 0x09C98323, 0xDDD04F36, 0x75E3D71C, 0xA1FA1B09,
  0xD52DB7B4, 0x01347BA1, 0xA907E38B, 0x7D1E2F9E, 0x2D791FCA, 0xF960D3DF, 0x51534BF5, 0x854A87E0, 0xB8FC128F,
  0x6CE5DE9A, 0xC4D646B0, 0x10CF8AA5, 0x40A8BAF1, 0x94B176E4, 0x3C82EECE, 0xE89B22DB, 0x9C4C8E66, 0x48554273,
  0xE066DA59, 0x347F164C, 0x64182618, 0xB001EA0D, 0x18327227, 0xCC2BBE32, 0x635F58F9, 0xB74694EC, 0x1F750CC6,
  0xCB6CC0D3, 0x9B0BF087, 0x4F123C92, 0xE721A4B8, 0x333868AD, 0x47EFC410, 0x93F60805, 0x3BC5902F, 0xEFDC5C3A,
  0xBFBB6C6E, 0x6BA2A07B, 0xC3913851, 0x1788F444, 0x2A3E612B, 0xFE27AD3E, 0x56143514, 0x820DF901, 0xD26AC955,
  0x06730540, 0xAE409D6A, 0x7A59517F, 0x0E8EFDC2, 0xDA9731D7, 0x72A4A9FD, 0xA6BD65E8, 0xF6DA55BC, 0x22C399A9,
  0x8AF00183, 0x5EE9CD96, 0x37239AAF, 0xE33A56BA, 0x4B09CE90, 0x9F100285, 0xCF7732D1, 0x1B6EFEC4, 0xB35D66EE,
  0x6744AAFB, 0x13930646, 0xC78ACA53, 0x6FB95279, 0xBBA09E6C, 0xEBC7AE38, 0x3FDE622D, 0x97EDFA07, 0x43F43612,
  0x7E42A37D, 0xAA5B6F68, 0x0268F742, 0xD6713B57, 0x86160B03, 0x520FC716, 0xFA3C5F3C, 0x2E259329, 0x5AF23F94,
  0x8EEBF381, 0x26D86BAB, 0xF2C1A7BE, 0xA2A697EA, 0x76BF5BFF, 0xDE8CC3D5, 0x0A950FC0, 0xA5E1E90B, 0x71F8251E,
  0xD9CBBD34, 0x0DD27121, 0x5DB54175, 0x89AC8D60, 0x219F154A, 0xF586D95F, 0x815175E2, 0x5548B9F7, 0xFD7B21DD,
  0x2962EDC8, 0x7905DD9C, 0xAD1C1189, 0x052F89A3, 0xD13645B6, 0xEC80D0D9, 0x38991CCC, 0x90AA84E6, 0x44B348F3,
  0x14D478A7, 0xC0CDB4B2, 0x68FE2C98, 0xBCE7E08D, 0xC8304C30, 0x1C298025, 0xB41A180F, 0x6003D41A, 0x3064E44E,
  0xE47D285B, 0x4C4EB071, 0x98577C64, 0xC6BEB1F2, 0x12A77DE7, 0xBA94E5CD, 0x6E8D29D8, 0x3EEA198C, 0xEAF3D599,
  0x42C04DB3, 0x96D981A6, 0xE20E2D1B, 0x3617E10E, 0x9E247924, 0x4A3DB531, 0x1A5A8565, 0xCE434970, 0x6670D15A,
  0xB2691D4F, 0x8FDF8820, 0x5BC64435, 0xF3F5DC1F, 0x27EC100A, 0x778B205E, 0xA392EC4B, 0x0BA17461, 0xDFB8B874,
  0xAB6F14C9, 0x7F76D8DC, 0xD74540F6, 0x035C8CE3, 0x533BBCB7, 0x872270A2, 0x2F11E888, 0xFB08249D, 0x547CC256,
  0x80650E43, 0x28569669, 0xFC4F5A7C, 0xAC286A28, 0x7831A63D, 0xD0023E17, 0x041BF202, 0x70CC5EBF, 0xA4D592AA,
  0x0CE60A80, 0xD8FFC695, 0x8898F6C1, 0x5C813AD4, 0xF4B2A2FE, 0x20AB6EEB, 0x1D1DFB84, 0xC9043791, 0x6137AFBB,
  0xB52E63AE, 0xE54953FA, 0x31509FEF, 0x996307C5, 0x4D7ACBD0, 0x39AD676D, 0xEDB4AB78, 0x45873352, 0x919EFF47,
  0xC1F9CF13, 0x15E00306, 0xBDD39B2C, 0x69CA5739
};

static const uint32_t crc32_table2[] = {
  0x00000000U, 0x77073096U, 0xee0e612cU, 0x990951baU, 0x076dc419U,
  0x706af48fU, 0xe963a535U, 0x9e6495a3U, 0x0edb8832U, 0x79dcb8a4U, 0xe0d5e91eU, 0x97d2d988U, 0x09b64c2bU,
  0x7eb17cbdU, 0xe7b82d07U, 0x90bf1d91U, 0x1db71064U, 0x6ab020f2U, 0xf3b97148U, 0x84be41deU, 0x1adad47dU,
  0x6ddde4ebU, 0xf4d4b551U, 0x83d385c7U, 0x136c9856U, 0x646ba8c0U, 0xfd62f97aU, 0x8a65c9ecU, 0x14015c4fU,
  0x63066cd9U, 0xfa0f3d63U, 0x8d080df5U, 0x3b6e20c8U, 0x4c69105eU, 0xd56041e4U, 0xa2677172U, 0x3c03e4d1U,
  0x4b04d447U, 0xd20d85fdU, 0xa50ab56bU, 0x35b5a8faU, 0x42b2986cU, 0xdbbbc9d6U, 0xacbcf940U, 0x32d86ce3U,
  0x45df5c75U, 0xdcd60dcfU, 0xabd13d59U, 0x26d930acU, 0x51de003aU, 0xc8d75180U, 0xbfd06116U, 0x21b4f4b5U,
  0x56b3c423U, 0xcfba9599U, 0xb8bda50fU, 0x2802b89eU, 0x5f058808U, 0xc60cd9b2U, 0xb10be924U, 0x2f6f7c87U,
  0x58684c11U, 0xc1611dabU, 0xb6662d3dU, 0x76dc4190U, 0x01db7106U, 0x98d220bcU, 0xefd5102aU, 0x71b18589U,
  0x06b6b51fU, 0x9fbfe4a5U, 0xe8b8d433U, 0x7807c9a2U, 0x0f00f934U, 0x9609a88eU, 0xe10e9818U, 0x7f6a0dbbU,
  0x086d3d2dU, 0x91646c97U, 0xe6635c01U, 0x6b6b51f4U, 0x1c6c6162U, 0x856530d8U, 0xf262004eU, 0x6c0695edU,
  0x1b01a57bU, 0x8208f4c1U, 0xf50fc457U, 0x65b0d9c6U, 0x12b7e950U, 0x8bbeb8eaU, 0xfcb9887cU, 0x62dd1ddfU,
  0x15da2d49U, 0x8cd37cf3U, 0xfbd44c65U, 0x4db26158U, 0x3ab551ceU, 0xa3bc0074U, 0xd4bb30e2U, 0x4adfa541U,
  0x3dd895d7U, 0xa4d1c46dU, 0xd3d6f4fbU, 0x4369e96aU, 0x346ed9fcU, 0xad678846U, 0xda60b8d0U, 0x44042d73U,
  0x33031de5U, 0xaa0a4c5fU, 0xdd0d7cc9U, 0x5005713cU, 0x270241aaU, 0xbe0b1010U, 0xc90c2086U, 0x5768b525U,
  0x206f85b3U, 0xb966d409U, 0xce61e49fU, 0x5edef90eU, 0x29d9c998U, 0xb0d09822U, 0xc7d7a8b4U, 0x59b33d17U,
  0x2eb40d81U, 0xb7bd5c3bU, 0xc0ba6cadU, 0xedb88320U, 0x9abfb3b6U, 0x03b6e20cU, 0x74b1d29aU, 0xead54739U,
  0x9dd277afU, 0x04db2615U, 0x73dc1683U, 0xe3630b12U, 0x94643b84U, 0x0d6d6a3eU, 0x7a6a5aa8U, 0xe40ecf0bU,
  0x9309ff9dU, 0x0a00ae27U, 0x7d079eb1U, 0xf00f9344U, 0x8708a3d2U, 0x1e01f268U, 0x6906c2feU, 0xf762575dU,
  0x806567cbU, 0x196c3671U, 0x6e6b06e7U, 0xfed41b76U, 0x89d32be0U, 0x10da7a5aU, 0x67dd4accU, 0xf9b9df6fU,
  0x8ebeeff9U, 0x17b7be43U, 0x60b08ed5U, 0xd6d6a3e8U, 0xa1d1937eU, 0x38d8c2c4U, 0x4fdff252U, 0xd1bb67f1U,
  0xa6bc5767U, 0x3fb506ddU, 0x48b2364bU, 0xd80d2bdaU, 0xaf0a1b4cU, 0x36034af6U, 0x41047a60U, 0xdf60efc3U,
  0xa867df55U, 0x316e8eefU, 0x4669be79U, 0xcb61b38cU, 0xbc66831aU, 0x256fd2a0U, 0x5268e236U, 0xcc0c7795U,
  0xbb0b4703U, 0x220216b9U, 0x5505262fU, 0xc5ba3bbeU, 0xb2bd0b28U, 0x2bb45a92U, 0x5cb36a04U, 0xc2d7ffa7U,
  0xb5d0cf31U, 0x2cd99e8bU, 0x5bdeae1dU, 0x9b64c2b0U, 0xec63f226U, 0x756aa39cU, 0x026d930aU, 0x9c0906a9U,
  0xeb0e363fU, 0x72076785U, 0x05005713U, 0x95bf4a82U, 0xe2b87a14U, 0x7bb12baeU, 0x0cb61b38U, 0x92d28e9bU,
  0xe5d5be0dU, 0x7cdcefb7U, 0x0bdbdf21U, 0x86d3d2d4U, 0xf1d4e242U, 0x68ddb3f8U, 0x1fda836eU, 0x81be16cdU,
  0xf6b9265bU, 0x6fb077e1U, 0x18b74777U, 0x88085ae6U, 0xff0f6a70U, 0x66063bcaU, 0x11010b5cU, 0x8f659effU,
  0xf862ae69U, 0x616bffd3U, 0x166ccf45U, 0xa00ae278U, 0xd70dd2eeU, 0x4e048354U, 0x3903b3c2U, 0xa7672661U,
  0xd06016f7U, 0x4969474dU, 0x3e6e77dbU, 0xaed16a4aU, 0xd9d65adcU, 0x40df0b66U, 0x37d83bf0U, 0xa9bcae53U,
  0xdebb9ec5U, 0x47b2cf7fU, 0x30b5ffe9U, 0xbdbdf21cU, 0xcabac28aU, 0x53b39330U, 0x24b4a3a6U, 0xbad03605U,
  0xcdd70693U, 0x54de5729U, 0x23d967bfU, 0xb3667a2eU, 0xc4614ab8U, 0x5d681b02U, 0x2a6f2b94U, 0xb40bbe37U,
  0xc30c8ea1U, 0x5a05df1bU, 0x2d02ef8dU
};
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/

/** Private function prototypes ----------------------------------------------*/

/** Private user code --------------------------------------------------------*/

/** Private application code -------------------------------------------------*/
/*******************************************************************************
*
*       Static code
*
********************************************************************************
*/

/** Public application code --------------------------------------------------*/
/*******************************************************************************
*
*       Public code
*
********************************************************************************
*/

utility::utility(QObject *parent)
  : QObject{parent}
{

}


/**
 * @brief modbusCRC calc with table
 *
 * @param data 数据
 * @param data_len crc区域数据长度
 * @return uint16_t crc结果
 */
uint16_t utility::get_modbus_crc16_with_tab(const uint8_t *data, uint16_t data_len)
{
  uint8_t ucCRCHi = 0xFF;
  uint8_t ucCRCLo = 0xFF;
  uint16_t index = 0;

  while(data_len--)
  {
    index = ucCRCLo ^ *(data++);
    //
    //
    ucCRCLo = (uint8_t)(ucCRCHi ^ auchCRCHi[index]);
    ucCRCHi = auchCRCLo[index];
  }
  return (uint16_t)( ucCRCHi << 8 | ucCRCLo );
}

/**
 * @brief modbusCRC calc with table
 *
 * @param data 数据与crc
 * @param data_len crc区域数据长度
 * @return bool crc结果正确
 */
bool utility::get_modbus_crc16_rsl_with_tab(const uint8_t *data, uint16_t data_len)
{
  uint8_t ucCRCHi = 0xFF;
  uint8_t ucCRCLo = 0xFF;
  uint16_t index = 0;
  uint16_t data_size = data_len;
  const uint8_t *data_ptr = data;

  while(data_size--)
  {
    index = ucCRCLo ^ *(data_ptr++);
    //
    //
    ucCRCLo = (uint8_t)(ucCRCHi ^ auchCRCHi[index]);
    ucCRCHi = auchCRCLo[index];
  }
  // return (uint16_t)( ucCRCHi << 8 | ucCRCLo );

  if(ucCRCLo == data[data_len] && ucCRCHi == data[data_len + 1])
  {
    return true;
  }
  else
  {
    return false;
  }
}

quint32 utility::get_crc32_with_tab(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum)
{
  quint32 i, j;
  for(j = 0; j < data_blk_size; j++)
  {
    i = ((int)(crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
    crc_accum = (crc_accum << 8) ^ crc32_table[i];
  }
  return crc_accum;
}

quint32 utility::get_crc32_with_tab1(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum)
{
  quint32 i, j;

  for(j = 0; j < data_blk_size; j++)
  {
    i = ((uint32_t)(crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
    crc_accum = (crc_accum << 8) ^ crc32_table1[i];
  }

  return crc_accum;
}

quint32 utility::get_crc32_with_tab2(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum)
{
  quint32 i, j;

  for(j = 0; j < data_blk_size; j++)
  {
    i = ((uint32_t)(crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
    crc_accum = (crc_accum << 8) ^ crc32_table2[i];
  }

  return crc_accum;
}

quint32 utility::get_crc32_with_tab2_for_upfw(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum, bool end)
{
  quint32 i, j;

  for(j = 0; j < data_blk_size; j++)
  {
    i = ((uint32_t)(crc_accum ^ data_blk_ptr[j]) & 0xff);
    crc_accum = (crc_accum >> 8) ^ crc32_table2[i];
  }

  if(true == end)
  {
    return crc_accum ^ 0xFFFFFFFFU;
  }
  return crc_accum;
}

static bool get_crc32_rsl_with_tab(const quint8 *data_blk_ptr, quint32 data_blk_size, quint32 crc_accum)
{
  uint32_t crc_val = utility::get_crc32_with_tab(data_blk_ptr, data_blk_size, crc_accum);

  uint32_t crc_val_ = 0;
  crc_val_ = data_blk_ptr[data_blk_size + 3];
  crc_val_ <<= 8;
  crc_val_ |= data_blk_ptr[data_blk_size + 2];
  crc_val_ <<= 8;
  crc_val_ |= data_blk_ptr[data_blk_size + 1];
  crc_val_ <<= 8;
  crc_val_ |= data_blk_ptr[data_blk_size];
  if(crc_val_ == crc_val)
  {
    return true;
  }
  return false;
}

void utility::delay_ms(int ms)
{
  QEventLoop loop;
  QTimer::singleShot(ms, &loop, &QEventLoop::quit);
  loop.exec();
}

quint32 utility::num_type_to_bytes(utility::NUM_TYPE_Typedef_t Type)
{
  quint32 unit_bytes = 0;
  switch(Type)
  {
    case CALTERAH_CFX_28BIT_DATA_TYPE:
    case CALTERAH_CFL_32BIT_DATA_TYPE:
    case INT32_DATA_TYPE:
    {
      unit_bytes = sizeof(qint32);
      break;
    }
    case COMPLEX_FLOAT_DATA_TYPE:
    {
      unit_bytes = sizeof(Complex_t);
      break;
    }
    case COMPLEX_INT16_DATA_TYPE:
    {
      unit_bytes = sizeof(Complex_I16_t);
      break;
    }
    case COMPLEX_INT32_DATA_TYPE:
      {
        unit_bytes = sizeof(Complex_I32_t);
        break;
      }
    case FLOAT32_DATA_TYPE:
    {
      unit_bytes = sizeof(float);
      break;
    }
    case INT16_DAYA_TYPE:
    {
      unit_bytes = sizeof(qint16);
      break;
    }
    case INT8_DATA_TYPE:
    {
      unit_bytes = sizeof(qint8);
      break;
    }
    case UINT32_DATA_TYPE:
    case FLOAT32_BIN_DATA_TYPE:
    {
      unit_bytes = sizeof(quint32);
      break;
    }
    case UINT16_DAYA_TYPE:
    {
      unit_bytes = sizeof(quint16);
      break;
    }
    case UINT8_DATA_TYPE:
    {
      unit_bytes = sizeof(quint8);
      break;
    }
    default:
      unit_bytes = 0;
      break;
  }
  return unit_bytes;
}

void utility::bytes_invert(quint8 *array, quint32 len)
{
  for(quint32 i = 0; i < (len / 2U); i++)
  {
    quint8 temp = array[i];
    array[i] = array[len - i - 1U];
    array[len - i - 1U] = temp;
  }
}

QString utility::data2str(const quint8 *data, NUM_TYPE_Typedef_t Type)
{
  if(nullptr == data)
  {
    return QString("0");
  }

  switch(Type)
  {
    case CALTERAH_CFX_28BIT_DATA_TYPE:
    case CALTERAH_CFL_32BIT_DATA_TYPE:
    case UINT32_DATA_TYPE:
      {
        quint32 val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case COMPLEX_FLOAT_DATA_TYPE:
      {
        float val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case COMPLEX_INT16_DATA_TYPE:
      {
        qint16 val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case COMPLEX_INT32_DATA_TYPE:
      {
        qint32 val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case FLOAT32_DATA_TYPE:
      {
        float val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case INT16_DAYA_TYPE:
      {
        qint16 val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case INT8_DATA_TYPE:
      {
        qint8 val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case INT32_DATA_TYPE:
      {
        qint32 val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case FLOAT32_BIN_DATA_TYPE:
      {
        float val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case UINT16_DAYA_TYPE:
      {
        quint16 val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    case UINT8_DATA_TYPE:
      {
        quint8 val = 0;
        memcpy(&val, data, sizeof(val));
        return QString::number(val);
      }
    default:
      return QString("0");
  }
}

quint32 utility::str2num(void *buf, const QStringList &num_str_list, utility::NUM_TYPE_Typedef_t Type, quint8 *unit_bytes)
{
  if(num_str_list.isEmpty())
  {
    return 0;
  }
  bool ok = false;
  quint32 num_size = 0;
  for(qint32 i = 0; i < num_str_list.size();)
  {
    switch(Type)
    {
      case INT32_DATA_TYPE:
      {
        qint32 *ptr = (qint32 *)buf;
        ptr[num_size] = (qint32)num_str_list.value(i).toInt(&ok);
        i += 1;
        if(false == ok)
        {
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(qint32);
        break;
      }
      case COMPLEX_FLOAT_DATA_TYPE:
      {
        Complex_t *ptr = (Complex_t *)buf;
        bool ok;
        ptr[num_size].real = num_str_list.value(i).toFloat(&ok);
        ptr[num_size].image = num_str_list.value(i + 1).toFloat(&ok);
        i += 2;
        if(ok == false)
        {
          ptr[i / 2].real = 0.0;
          ptr[i / 2].image = 0.0;
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(Complex_t);
        break;
      }
      case COMPLEX_INT16_DATA_TYPE:
      {
        Complex_I16_t *ptr = (Complex_I16_t *)buf;
        ptr[num_size].real = num_str_list.value(i).toShort(&ok);
        ptr[num_size].image = num_str_list.value(i + 1).toShort(&ok);
        i += 2;
        if(false == ok)
        {
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(Complex_I16_t);
        break;
      }
      case COMPLEX_INT32_DATA_TYPE:
        {
          Complex_I32_t *ptr = (Complex_I32_t *)buf;
          ptr[num_size].real = num_str_list.value(i).toInt(&ok);
          ptr[num_size].image = num_str_list.value(i + 1).toInt(&ok);
          i += 2;
          if(false == ok)
          {
            continue;
          }

          num_size += 1;
          *unit_bytes = sizeof(Complex_I32_t);
          break;
        }
      case FLOAT32_DATA_TYPE:
      {
        float *ptr = (float *)buf;
        ptr[num_size] = num_str_list.value(i).toFloat(&ok);
        i += 1;
        if(ok == false)
        {
          ptr[i] = 0.0;
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(float);
        break;
      }
      case INT16_DAYA_TYPE:
      {
        qint16 *ptr = (qint16 *)buf;
        ptr[num_size] = num_str_list.value(i).toShort(&ok);
        i += 1;
        if(false == ok)
        {
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(qint16);
        break;
      }
      case INT8_DATA_TYPE:
      {
        qint8 *ptr = (qint8 *)buf;
        ptr[num_size] = (qint8)num_str_list.value(i).toShort(&ok);
        i += 1;
        if(false == ok)
        {
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(qint8);
        break;
      }
      case CALTERAH_CFX_28BIT_DATA_TYPE:
      case CALTERAH_CFL_32BIT_DATA_TYPE:
      case UINT32_DATA_TYPE:
      case FLOAT32_BIN_DATA_TYPE:
      {
        quint32 *ptr = (quint32 *)buf;
        ptr[num_size] = (quint32)num_str_list.value(i).toUInt(&ok);
        i += 1;
        if(false == ok)
        {
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(quint32);
        break;
      }
      case UINT16_DAYA_TYPE:
      {
        quint16 *ptr = (quint16 *)buf;
        ptr[num_size] = num_str_list.value(i).toUShort(&ok);
        i += 1;
        if(false == ok)
        {
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(quint16);
        break;
      }
      case UINT8_DATA_TYPE:
      {
        quint8 *ptr = (quint8 *)buf;
        ptr[num_size] = (quint8)num_str_list.value(i).toUShort(&ok);
        i += 1;
        if(false == ok)
        {
          continue;
        }

        num_size += 1;
        *unit_bytes = sizeof(quint8);
        break;
      }
      default:
        *unit_bytes = 0;
        return 0;
    }
  }
  return num_size;
}

quint8 utility::get_data_sum(const quint8 *data, quint32 len)
{
  quint32 val = 0;
  for(quint32 i = 0; i < len; i++)
  {
    val += data[i];
  }
  return (quint8)val;
}

bool utility::get_sum_rsl(const quint8 *data, quint32 len)
{
  quint32 val = 0;
  for(quint32 i = 0; i < len; i++)
  {
    val += data[i];
  }
  if(data[len] == (quint8)val)
  {
    return true;
  }
  return false;
}

QString utility::line_data2split(const QString &line_data)
{
  QString line_str = line_data;
  /* 删除0x */
  QRegExp hex_remove_rx("0x");
  QString str = line_str.replace(hex_remove_rx, "");

  /* 替换逗号 */
  str = str.replace(",", " ");

  /* 替换冒号 */
  str = str.replace(":", " ");

  /* 匹配一个或多个空格字符，逗号字符，分割 */
  QRegExp split_rx("\\s+");
  QStringList data_list = str.split(split_rx, Qt::SkipEmptyParts);
  QString data = data_list.join(' ');
  return data;
}

QString utility::array2hexstr(const quint8 *data, quint32 len, const QString &split)
{
  QStringList data_list;
  for(quint32 i = 0; i < len; i++)
  {
    data_list.append(QString::number((int)data[i], 16));
  }
  return data_list.join(split);
}

/**
 * @brief 包号重复检测
 *
 * @param pRecord 记录句柄
 * @param Pack_Num 包号
 * @param Init_En 初始化使能，初始化使能时，仅进行初始化，
 * @param Check_En 检测使能，检测使能时检测，false时记录
 * @return 返回true代表重复包
 */
bool utility::Check_Current_Pack_Num_Is_Repeat(PACK_REPEAT_CHECK_Typedef_t *pRecord, uint32_t Pack_Num, \
                                             bool Init_En, bool Check_En)
{
  if(true == Init_En)
  {
    (void)memset(pRecord, 0, sizeof(PACK_REPEAT_CHECK_Typedef_t));
    return true;
  }

  if(true == Check_En)
  {
    /* 检测是否需要重置 */
    if((Pack_Num % EVEN_PACKAGE_REPEAT_CHECK_SIZE) == 0U)
    {
      (void)memset(pRecord->State_Record_Even, 0, sizeof(pRecord->State_Record_Even));
    }
    if(((Pack_Num + 1U) % ODD_PACKAGE_REPEAT_CHECK_SIZE) == 0U)
    {
      (void)memset(pRecord->State_Record_Odd, 0, sizeof(pRecord->State_Record_Odd));
    }

    /* 检测 */
    if(pRecord->State_Record_Even[Pack_Num % EVEN_PACKAGE_REPEAT_CHECK_SIZE] == 1U ||
        pRecord->State_Record_Odd[(Pack_Num + 1U) % ODD_PACKAGE_REPEAT_CHECK_SIZE] == 1U)
    {
      return true;
    }
    /* 未重复 */
    return false;
  }
  else
  {
    /* 记录 */
    pRecord->State_Record_Even[Pack_Num % EVEN_PACKAGE_REPEAT_CHECK_SIZE] = 1U;
    pRecord->State_Record_Odd[(Pack_Num + 1U) % ODD_PACKAGE_REPEAT_CHECK_SIZE] = 1U;
  }
  return true;
}

void utility::export_table2csv_file(QTableWidget *tableWidget, QString filePath)
{
  QFile file(filePath);
  if(false == file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    return;
  }

  QTextStream out(&file);
  out.setCodec("UTF-8");

  int rowCount = tableWidget->rowCount();
  int columnCount = tableWidget->columnCount();

  /* 写入表头 */
  for (int i = 0; i < columnCount; i++)
  {
    QTableWidgetItem *headerItem = tableWidget->horizontalHeaderItem(i);
    if(headerItem)
    {
      out << headerItem->text();
      if (i < columnCount - 1)
      {
        out << ",";
      }
    }
  }
  out << "\n";

  /* 写入数据 */
  for(int row = 0; row < rowCount; row++)
  {
    for(int col = 0; col < columnCount; col++)
    {
      QTableWidgetItem *item = tableWidget->item(row, col);
      if(item)
      {
        out << item->text();
        if (col < columnCount - 1)
        {
          out << ",";
        }
      }
    }
    out << "\n";
  }

  file.close();
}

/**
 * @brief 16进制格式调试打印
 *
 * @param msg 数据
 * @param msg_len 数据长度
 */
void utility::debug_print(const uint8_t *msg, uint32_t msg_len, QString prefix_str)
{ 
  QString hex_str = prefix_str;
  for(uint32_t i = 0; i < msg_len; i++)
  {
    hex_str += QString::asprintf("%02X ", msg[i]);
  }
  qDebug() << hex_str;

//    for(uint32_t i = 0; i < msg_len; i++)
//    {
//      fprintf(stdout, "%02X ", msg[i]);
//      fflush(stdout);
//    }
//    printf("\n");
//    fflush(stderr);
//    fflush(stdout);
}

QString utility::unicode_to_gb2312(const QString &unicode_str)
{
  QTextCodec *gb2312_codec = QTextCodec::codecForName("gb2312");
  QByteArray byte_gb2312 = gb2312_codec->fromUnicode(unicode_str);
  QString gb2312_str(byte_gb2312);
  return gb2312_str;
}

QString utility::gb2312_to_unicode(const QString &gb2312_str)
{
  QTextCodec *gb2312_codec = QTextCodec::codecForName("gb2312");
  QTextDecoder *gb2312_decoder = gb2312_codec->makeDecoder();
  QString unicode_str = gb2312_decoder->toUnicode(gb2312_str.toStdString().data(), gb2312_str.toStdString().size());
  delete gb2312_decoder;
  return unicode_str;
}

/******************************** End of file *********************************/
