//  Ví dụ phương pháp kết xuất âm thanh đơn giản
//  Phiên Bản 1.12
//  Phát hành 2560/02/05
//  Khởi đầu 2558/07/20

//  Hồ Nhựt Châu  su_huynh@yahoo.com
//谐波
// AmNhacTruong
//  - Thông tin Âm Thanh
//     +-------------------
//     |  - soLuongMauVat;   // số lượng mẫu vật
//     |  - tocDoMauVat;     // tốc độ mẫu vật
//     |  - soLuongMauVatGiay;  // số lượng mẫu vật/giây
//     |  - soLuongKenh;    // số lượng kênh
//     |  - mangMauVatKenh0;       // mảng mẫu vật kênh
//     |  - mangMauVatKenh1;       // mảng mẫu vật kênh
//     |  -               // dữ liệu kênh
//     |  - mảng mẫu Vật
//     +-------------------
//
//  - Danh Sách Làn
//     +-------------------
// làn |   - đàn
//     |   - mảng Nốt
//     +-------------------
//     +-------------------
// làn |   - đàn
//     |   - mảng Nốt
//     +-------------------
// ...

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>

#define hSAI  0
#define hDUNG 1

#define kSO_LUONG_NOT_TOI_DA 16384
#define kSO_LUONG_DAN_TOI_DA   256

#define kSO_NGUYEN_TOI_DA 2147483647.0f // <--- cho 64 bit?  // 32767.0f // <--- cho 32 bit?

#define hKIEU_PCM_16  0x0001
#define hKIEU_FLOAT   0x0003

#define hTOC_DO_MAU_VAT_MAC_DINH_22257 22257.0f
#define hTOC_DO_MAU_VAT_MAC_DINH_22050 22050.0f
#define hTOC_DO_MAU_VAT_MAC_DINH_44100 44100.0f
#define hTOC_DO_MAU_VAT_MAC_DINH_48000 48000.0f

#define hTAN_SO_CO_BAN 440.0f



#define kGIAI_DIEU_5       5
#define kGIAI_DIEU_12_TAY 12

// Giai Điệu Tây  (cỡ bản)*2^(n/12)
#define kNOT_TAY_0  1.000000f  // C  Đô
#define kNOT_TAY_1  1.059463f  //  ---
#define kNOT_TAY_2  1.122462f  // D  Rê
#define kNOT_TAY_3  1.189207f  //  ---
#define kNOT_TAY_4  1.259921f  // E  Mi
#define kNOT_TAY_5  1.334840f  // F  Pha
#define kNOT_TAY_6  1.414214f  //  ---
#define kNOT_TAY_7  1.498307f  // G  Xô
#define kNOT_TAY_8  1.587401f  //  ---
#define kNOT_TAY_9  1.681793f  // A  La
#define kNOT_TAY_10 1.781793f  //  ---
#define kNOT_TAY_11 1.887749f  // B  Ti



enum {
   kLOAI_SONG__SIN,
   kLOAI_SONG__TAM_GIAC,
   kLOAI_SONG__RANG_CUA_XUONG,
   kLOAI_SONG__RANG_CUA_LEN,
   kLOAI_SONG__VUONG,
   kLOAI_SONG__HUYEN_NAO_TRANG,
   kLOAI_SONG__CHINH_AM_LUONG,
   kLOAI_SONG__CHINH_TAN_SO,
   kLOAI_SONG__NHAN_TAN_SO,
};


/* Vectơ2C - cũng xài cho điểm */
typedef struct {
   float x;   // tọa đồ  - thời gian
   float y;   //         - âm lượng
} Vecto2C;


//
// đoạn tần số ---> |  đoạn -1          |   đoạn 1      |     đoạn 2   | ...
//                  | số nốt trong đoạn |               |              |
// tần số  ---->   0,5                 1,0             2,0            4,0
// tương đối cơ sở
//                             tần số cơ sở
typedef struct {
   // ---- cho tần số
   unsigned char giaiDieu;  // giai điệu của nốt
   char doan;               // đoàn tấn số của nốt
   char so;                 // số nốt tron đoàn tần số
   // ---- cho thời gian hát
   float thoiHat;           // thời hát
   float hatBaoLau;         // hát bao lâu
   // ---- âm lượng
   float amLuong;           // âm lượng
   // ---- vị trí
   float viTriTraiTrai;     // trái
   float viTriTraiPhai;     // phải
} Not;

// =========== BEZIER
/* Bezier Một Chiều */   // cho vật chuyển động
typedef struct {
   Vecto2C diem0;  // danh sách điểm quản trị
   Vecto2C diem1;
   Vecto2C diem2;
   Vecto2C diem3;
} Bezier;


/* Bao Bì */
typedef struct {   // CHÚ Ý: có nhiều bao bì không xài mỗi cái
   float batDau;    // khi nào bắt đầu phát âm (giây)
   float ha;        // hạ (giây)
   float giua;      // khi nào bắt đầu khúc giữa (giây)
   float ketThuc;   // khi nào bắt đầu khúc kêt thúc (giây)
   float amLuong;   // âm lượng cho bao bì
   unsigned char loai;  // loại
} BaoBi;

#define kBAO_BI__KHONG    0 // không đổi âm thanh
#define kBAO_BI__DEU      1 // âm thanh hát đều trừ đầu và kết thúc
#define kBAO_BI__MOC0     2 // lên lẹ hạ chậm
#define kBAO_BI__MOC1     3 // lên lẹ hạ chậm
#define kBAO_BI__DOI      4 // lên chậm, hạ chậm
#define kBAO_BI__CHUAN    5 // lên chuẩn


/* ======= BAO BÌ ======== */
/* Đàn Sóng */
typedef struct {
   unsigned char soLuongTanSo;  // số lượng tần số
   float mangTanSo[10];            // mảng tần số; tương đối với tần số nốt
   unsigned char soLuongBaoBi;  // số lượng bao bì

   BaoBi mangBaoBi[10];            // mảng bao bì
   unsigned char baoBiTuongDoi;  // phóng to bao bì tùy bề dài nốt
} DanSong;

/* Đàn Đổi Âm Lượng */
typedef struct {
   unsigned char soLuongTanSo;  // số lượng tần số
   float tanSo;            // tần số (tần số chánh)
   float tanSoChinh;       // tần số chỉnh tần số chánh
   float amLuongChinh;     // âm lượng chỉnh

   BaoBi baoBi;    // bao bì
   unsigned char baoBiTuongDoi;  // phóng to bao bì tùy bề dài nốt
//   float thoiKetThuc;    // thời bắt đầu giảm âm lượng đến 0 trước kết thúc nốt
} DanDoiAmLuong;

/* Đàn Đổi Tần Số */
typedef struct {
   float tanSo[4];               // tần số
   float tanSoChinh[4];          // tần số chỉnh tần số chánh
   float amLuongChinhDau[4];     // âm lượng chỉnh đầu
   float amLuongChinhCuoi[4];    // âm lượng chỉnh cuối
   unsigned char soLuongBaoBi;  // số lương bao bì

   BaoBi baoBi[4];         // bao bì
   BaoBi baoBiChinh[4];    // bao bì chỉnh (cho chỉnh tần số)
   unsigned char baoBiTuongDoi;  // phóng to bao bì tùy bề dài nốt
} DanDoiTanSo;

/* Đàn Đổi Tần Số Cặp */
typedef struct {
   float tanSoChinh0[4];          // tần số chỉnh tần số chánh
   float tanSoChinh1[4];          // tần số chỉnh tần số chánh
   float amLuongChinh0[4];     // âm lượng chỉnh đầu
   float amLuongChinh1[4];    // âm lượng chỉnh cuối
   unsigned char soLuongBaoBi;  // số lương bao bì

   BaoBi baoBi[4];          // bao bì
   BaoBi baoBiChinh0[4];    // bao bì chỉnh (cho chỉnh tần số)
   BaoBi baoBiChinh1[4];    // bao bì chỉnh (cho chỉnh tần số)
   unsigned char baoBiTuongDoi;  // phóng to bao bì tùy bề dài nốt
} DanDoiTanSoCap;

/* Đàn Huyên Náo Trắng */
typedef struct {
   float amLuong;        // âm lượng
   BaoBi baoBi;          // bao bì
   unsigned char baoBiTuongDoi;  // phóng to bao bì tùy bề dài nốt
} DanHuyenNaoTrang;

/* Đàn Đổi Bảng Sóng */
typedef struct {
   unsigned char soLuongBangSong; // số lương bảnh sóng
   unsigned short mangBangSong;  // mảng bàng sóng
   unsigned int giaTriDau;       // giá trị ầu cho bảng
   unsigned char giaiNangCap;    // giải nâng cấp bảng
      
   unsigned char soLuongBuoc;  // số lượng bước, tương đương như hài độ
   int mangBuoc[16];  // mảng bước, tương đương như hài độ, 16 - 16 phẩy tĩnh (16 bit cho số nguyên, 16 bit cho phân số)
   unsigned short suCham;  // sự chậm giữa các nốt
   unsigned short thoiGianHat;  // thời gian hát
      
   unsigned char bangSong[512];
   unsigned short thoiGianNangCap;   // đời bao lâu trước nâng cấp bảng sóng
      
} DanBangSong;


typedef union {
   DanSong danSong;                    /* Đàn Sóng */
   DanDoiAmLuong danDoiAmLuong;        /* Đàn Đổi Âm Lượng */
   DanDoiTanSo danDoiTanSo;            /* Đàn Đổi Tần Số */
   DanDoiTanSoCap danDoiTanSoCap;      /* Đàn Đổi Tần Số Cấp */
   DanBangSong danBangSong;            /* Đàn Bảng Sóng */
   DanHuyenNaoTrang danHuyenNaoTrang;  /* Đàn Huyên Náo Trắng */
} KieuDan;


// ==== Đàn
typedef struct {
   unsigned char ten[256];  // tên
   KieuDan kieuDan;         // kiểu đàn
   unsigned char loai;      // loại
} Dan;

#define kKIEU_DAN__SONG         0 // không đổi âm thanh
#define kKIEU_DAN__DOI_AM_LUONG 1 // âm thanh hát đều trừ đầu và kết thúc
#define kKIEU_DAN__DOI_TAN_SO   2 // lên lẹ hạ chậm
#define kKIEU_DAN__DOI_TAN_SO_CAP  3 // lên lẹ hạ chậm
#define kKIEU_DAN__HUYEN_NAO    4 // lên lẹ hạ chậm
#define kKIEU_DAN__BANG_SONG    5 // lên lẹ hạ chậm

/* Làn */
typedef struct {
   Dan dan;             // đàn
   Not mangNot[65536];   // mảng nốt cho đàn
   unsigned short soLuongNot;
} Lan;

/* dữ liệu âm thanh */
typedef struct {
   unsigned int soLuongMauVat;   // số lượng mẫu vật trong một kênh
   unsigned int tocDoMauVat;     // tốc độ mẫu vật cho các kênh
   unsigned int soLuongMauVatGiay;  // số lượng mẫu vật/giây
   unsigned char soLuongKenh;    // số lượng kênh
   float *mangMauVatKenh0;       // mảng mẫu vật kênh
   float *mangMauVatKenh1;       // mảng mẫu vật kênh
} DuLieuAmThanh;


/* Am Nhạc Trường */
typedef struct {
   char ten[256];                // tên
   unsigned char soLuongLan;     // sô lượng làn
   Lan *danhSachLan;             // danh sách làn
   DuLieuAmThanh duLieuAmThanh;  // dữ liệu âm thanh (cho kết xuất âm thanh)

   float tanSoCoSo;        // tần số cơ sử cho hát âm thanh (Hz)
   float tongThoiGianHat;  // tông thời gian hát
   unsigned char soPhimTruong;
} AmNhacTruong;



/* Tính Tân Số Nốt Nhạc */
float tinhTanSoNot( Not *notNhac );
void ketXuatAmNhac( AmNhacTruong *amNhacTruong );
void ketXuat_nhiKenh( DuLieuAmThanh *duLieuAmThanh, Lan *lan );

/* Đơn Vị Hóa */
void donViHoaMauVatNhiKenh( DuLieuAmThanh *duLieuAmThanh );

/* Lập Tạp Đàn */
//unsigned char lapTapDan0();

DanSong datDan_danSongNgauNhien();
DanSong datDan_danDay0();
DanSong datDan_danDay1();
DanSong datDan_danSong0();
DanDoiAmLuong datDan_chuong0();
DanDoiAmLuong datDan_doiAmLuong1();
DanDoiTanSo datDan_chuong1();
DanDoiTanSo datDan_ghen0();
DanDoiTanSo datDan_ghen1();
DanDoiTanSo datDan_ghen2();
DanDoiTanSo datDan_doiTanSo4();
DanDoiTanSo datDan_doiTanSo5();
DanHuyenNaoTrang datDan_huyenNao0();


/* Lưu Âm Thanh */
void luuAmThanh( char *tenTep, DuLieuAmThanh *duLieuAmThanh, unsigned char kieuDuLieu );
void luuThongTinDau( FILE *tep, DuLieuAmThanh *duLieuAmThanh, unsigned short kieuDuLieu );
void luuDuLieuSoNguyen16bit_nhiKenh( FILE *tep, DuLieuAmThanh *duLieuAmThanh );
void luuDuLieuSoThat32bit_nhiKenh( FILE *tep, DuLieuAmThanh *duLieuAmThanh );

/* Các bao bì */   // hết thời gian là giây từ bắt đầu hát nốt
float tinhAmLuong_baoBiDeu( float hatBaoLau, float batDau, float thoiGianTuongDoi, float thoiGiua, float thoiGiam );
float tinhAmLuong_baoBiMoc0( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau );
float tinhAmLuong_baoBiMoc1( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau, float thoiGiua );
float tinhAmLuong_baoBiDoi( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau, float thoiGiua );
float tinhAmLuong_baoBiDoc( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau );
float tinhAmLuong_baoBiChuan( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau, float thoiHa, float thoiGiua, float thoiKetThuc );

/* Hát Bao Bì */
float hatBaoBi( unsigned char loaiBaoBi, BaoBi *baoBi, float thoiHat, float thoiGianTuongDoi );

/* Bezier */
float tinhViTriBezierX( Bezier *bezier, float thamSo );
float tinhViTriBezierY( Bezier *bezier, float thamSo );
float timThamSoChoX( Bezier *bezier, float x );
float tinhAmLuongTuBaoBi( BaoBi *baoBi, float thoiGianTrongNot, float thoiHat, unsigned char baoBiTuongDoi );

#pragma mark Đọc Tham Số Dòng Lệnh
void docThamSoPhimTruong( int argc, char **argv, unsigned int *soPhimTruong );

/* Đặt Các Nốt Nhạc */
AmNhacTruong datAmNhacTruong0();
AmNhacTruong datAmNhacTruong1();
AmNhacTruong datAmNhacTruong2();

void boAmTruong_nhiKenh( AmNhacTruong *amNhacTruong );


int main( unsigned int argc, char **argv ) {

   srand( time(0) );

   // ---- đặt âm trường, số lượng kênh, tốc độ mẩu vật, v.v.
   AmNhacTruong amNhacTruong;
   unsigned int soPhimTruong = 0;
   docThamSoPhimTruong( argc, argv, &soPhimTruong );
   
   if( soPhimTruong == 0 )
      amNhacTruong = datAmNhacTruong0( argc, argv );
   else if( soPhimTruong == 1 )
      amNhacTruong = datAmNhacTruong1( argc, argv );
   else if( soPhimTruong == 2 )
      amNhacTruong = datAmNhacTruong2( argc, argv );
   
   // ---- giữ số phim trường
   amNhacTruong.soPhimTruong = soPhimTruong;

   char tenAmNhac[256];
   sprintf( tenAmNhac, "ÂmNhạc_%02d.wav", amNhacTruong.soPhimTruong );
   
   // ---- kết xuất nhạc
   ketXuatAmNhac( &amNhacTruong );

   // ---- đơn vị hóa
   donViHoaMauVatNhiKenh( &(amNhacTruong.duLieuAmThanh) );

   // ---- lưư tệp âm thanh (WAV)

   luuAmThanh( tenAmNhac, &(amNhacTruong.duLieuAmThanh), hKIEU_PCM_16 );
   printf( "Kết Xuất: %s\n", tenAmNhac );
   // ---- thả trí nhớ
   boAmTruong_nhiKenh( &amNhacTruong );

   return 1;
}

#pragma mark ---- Kết Xuất
void ketXuatAmNhac( AmNhacTruong *amNhacTruong ) {
   
   // ---- số lượng đàn
   unsigned char soLuongLan = amNhacTruong->soLuongLan;

   unsigned char soLan = 0;
   while( soLan < soLuongLan ) {
      // ---- mỗi lan chỉ có một đàn
      ketXuat_nhiKenh( &(amNhacTruong->duLieuAmThanh), &(amNhacTruong->danhSachLan[soLan]) );

      soLan++;
   }
   
}

#pragma mark ---- Kết Xuất Đàn Sóng
float tinhGiaTriNotDanSong( float tanSoNot, DanSong *danSong, float thoiGianTrongNot, float hatBaoLau );
float tinhGiaTriNotDanDoiAmLuong( float tanSoNot, DanDoiAmLuong *danDoiAmLuong, float thoiGianTrongNot, float hatBaoLau );
float tinhGiaTriNotDanDoiTanSo( float tanSoNot, DanDoiTanSo *danDoiTanSo, float thoiGianTrongNot, float hatBaoLau );
float tinhGiaTriNotDanDoiTanSoCap( float tanSoNot, DanDoiTanSoCap *danDoiTanSoCap, float thoiGianTrongNot, float hatBaoLau );
float tinhGiaTriNotDanHuyenNaoTrang( DanHuyenNaoTrang *danHuyenNaoTrang, float thoiGianTrongNot, float hatBaoLau );

void ketXuat_nhiKenh( DuLieuAmThanh *duLieuAmThanh, Lan *lan ) {
   
   unsigned int soLuongMauVat = duLieuAmThanh->soLuongMauVat;
//   printf( "soLuongMauVat %d   %5.3f\n", soLuongMauVat, soLuongMauVat/hTOC_DO_MAU_VAT_MAC_DINH_44100 );
   
   // ---- lấy mảng nốt
   Not *mangNot = lan->mangNot;

   // ---- số lượng nốt
   unsigned short soLuongNot = lan->soLuongNot;
   unsigned short soNot = 0;
   float buocThoiGian = 1.0f/duLieuAmThanh->tocDoMauVat;
   
   while( soNot < soLuongNot ) {
      // ---- lấy nốt nhạc
      Not *not = &(mangNot[soNot]);
      
//      printf( "%d not->amLuong %5.3f\n", soNot, not->amLuong );
      
      // ---- tính tần số nốt
      float tanSoNot = tinhTanSoNot( not );
      
      // ---- số mẫu vật bắt đầu hát
      unsigned int soMauVat = duLieuAmThanh->tocDoMauVat*not->thoiHat;
      unsigned int soLuongMauVatKetXuat = duLieuAmThanh->tocDoMauVat*not->hatBaoLau;
      
//      printf( "%d tanSo %5.3f  soMauVat %d   soLuongMauVatKetXuat %d\n", soNot, tanSoNot, soMauVat, soLuongMauVatKetXuat );
      // ---- thời gian trong nốt
      float thoiGianTrongNot = 0.0f;

      // ---- chỉ cần kết xuất nếu 
      if( soMauVat < soLuongMauVat ) {
         unsigned int soMauVatKetThuc = soMauVat + soLuongMauVatKetXuat;

         // ---- đừng cho ra ngoài mảng mẫu vật
         if( soMauVatKetThuc > soLuongMauVat)
            soMauVatKetThuc = soLuongMauVat;

         while( soMauVat < soMauVatKetThuc ) {

               // ---- đàn cho làn này
            unsigned char loaiDan = lan->dan.loai;
            float giaTri = 0.0f;
//            printf( "loaiDan %d soMauVat %d\n", loaiDan, soMauVat );

            if( loaiDan == kKIEU_DAN__SONG ) {
               DanSong *danSong = &(lan->dan.kieuDan.danSong);
               giaTri = not->amLuong*tinhGiaTriNotDanSong( tanSoNot, danSong, thoiGianTrongNot, not->hatBaoLau );
//               printf( "giaTri %5.3f\n", giaTri );
            }
            else if( loaiDan == kKIEU_DAN__DOI_AM_LUONG ) {
               DanDoiAmLuong *danDoiAmLuong = &(lan->dan.kieuDan.danDoiAmLuong);
               giaTri = not->amLuong*tinhGiaTriNotDanDoiAmLuong( tanSoNot, danDoiAmLuong, thoiGianTrongNot, not->hatBaoLau );
            }
            else if( loaiDan == kKIEU_DAN__DOI_TAN_SO ) {
               DanDoiTanSo *danDoiTanSo = &(lan->dan.kieuDan.danDoiTanSo);
               giaTri = not->amLuong*tinhGiaTriNotDanDoiTanSo( tanSoNot, danDoiTanSo, thoiGianTrongNot, not->hatBaoLau );
            }
            else if( loaiDan == kKIEU_DAN__DOI_TAN_SO_CAP ) {
               DanDoiTanSoCap *danDoiTanSoCap = &(lan->dan.kieuDan.danDoiTanSoCap);
               giaTri = not->amLuong*tinhGiaTriNotDanDoiTanSoCap( tanSoNot, danDoiTanSoCap, thoiGianTrongNot, not->hatBaoLau );
            }
            else { // kLOAI_SONG__HUYEN_NAO_TRANG
               DanHuyenNaoTrang *danHuyenNaoTrang = &(lan->dan.kieuDan.danHuyenNaoTrang);
               giaTri = not->amLuong*tinhGiaTriNotDanHuyenNaoTrang( danHuyenNaoTrang, thoiGianTrongNot, not->hatBaoLau );
            }
            // ---- ghi lưu trong hai kênh
            duLieuAmThanh->mangMauVatKenh0[soMauVat] += giaTri*not->viTriTraiTrai;
            duLieuAmThanh->mangMauVatKenh1[soMauVat] += giaTri*not->viTriTraiPhai;
            soMauVat++;
            thoiGianTrongNot += buocThoiGian;
         }
      }
      
      soNot++;
   }

}

float tinhGiaTriNotDanSong( float tanSoNot, DanSong *danSong, float thoiGianTrongNot, float hatBaoLau ) {
   
   unsigned char soLuongTanSo = danSong->soLuongTanSo;
   float *mangTanSo = danSong->mangTanSo;            // mảng tần số
   unsigned char soLuongBaoBi = danSong->soLuongBaoBi;  // số lượng bao bì
   BaoBi *mangBaoBi = danSong->mangBaoBi;    // mảng bao bì
   unsigned char baoBiTuongDoi = danSong->baoBiTuongDoi;
   
//   float thoiKetThuc;    // thời bắt đầu giảm âm lượng đến 0 trước kết thúc nốt
   unsigned short soTanSo = 0;
   float ketQua = 0.0f;
   
   while( soTanSo < soLuongTanSo ) {
      float amLuong = tinhAmLuongTuBaoBi( &(mangBaoBi[soTanSo]), thoiGianTrongNot, hatBaoLau, baoBiTuongDoi );
//      printf( "%d %5.3f   amLuong %5.3f\n", soTanSo, mangTanSo[soTanSo], amLuong );
      ketQua += amLuong*sinf( tanSoNot*thoiGianTrongNot*mangTanSo[soTanSo] );
      soTanSo++;
   }

   return ketQua; // (float)rand()/(float)0x7ffffffff;
}


float tinhGiaTriNotDanDoiAmLuong( float tanSoNot, DanDoiAmLuong *danDoiAmLuong, float thoiGianTrongNot, float hatBaoLau ) {
   
   float tanSo = danDoiAmLuong->tanSo;            // mảng tần số
   float tanSoChinh = danDoiAmLuong->tanSoChinh;            // mảng tần số
   float amLuongChinh = danDoiAmLuong->amLuongChinh;            // mảng tần số

   BaoBi *baoBi = &(danDoiAmLuong->baoBi);    // bao bì
   unsigned char baoBiTuongDoi = danDoiAmLuong->baoBiTuongDoi;

      float amLuong = tinhAmLuongTuBaoBi( baoBi, thoiGianTrongNot, hatBaoLau, baoBiTuongDoi );
//      printf( "%d %5.3f   amLuong %5.3f\n", soTanSo, mangTanSo[soTanSo], amLuong );
      // duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*(1.0f + mangAmLuong[soTanSo*sinf(haiDoChinhTinh*thoiGian))*sinf(haiDoTinh*thoiGian);
     float ketQua = amLuong*(1.0f + amLuongChinh*sinf( tanSoNot*thoiGianTrongNot*tanSoChinh ))*sinf(tanSo*thoiGianTrongNot*tanSoNot );

   return ketQua;
}

float tinhGiaTriNotDanDoiTanSo( float tanSoNot, DanDoiTanSo *danDoiTanSo, float thoiGianTrongNot, float hatBaoLau ) {

   unsigned char soLuongBaoBi = danDoiTanSo->soLuongBaoBi;
   unsigned char soBaoBi = 0;
   float ketQua = 0.0f;
   while( soBaoBi < soLuongBaoBi ) {
   float tanSo = danDoiTanSo->tanSo[soBaoBi];            // tần số
   float tanSoChinh = danDoiTanSo->tanSoChinh[soBaoBi];            // tần số
   float amLuongChinhDau = danDoiTanSo->amLuongChinhDau[soBaoBi];            // tần số
   float amLuongChinhCuoi = danDoiTanSo->amLuongChinhCuoi[soBaoBi];            // tần số

   BaoBi *baoBi = &(danDoiTanSo->baoBi[soBaoBi]);    // bao bì
   BaoBi *baoBiChinh = &(danDoiTanSo->baoBiChinh[soBaoBi]);    // bao bì
      unsigned char baoBiTuongDoi = danDoiTanSo->baoBiTuongDoi;
   
   float amLuong = tinhAmLuongTuBaoBi( baoBi, thoiGianTrongNot, hatBaoLau, baoBiTuongDoi );
//   float amLuongChinh = amLuongChinhDau + (amLuongChinhCuoi - amLuongChinhDau)*thoiGianTrongNot;
   float amLuongChinh = amLuongChinhDau + (amLuongChinhCuoi - amLuongChinhDau)*tinhAmLuongTuBaoBi( baoBiChinh, thoiGianTrongNot,
                                                                                                  hatBaoLau, baoBiTuongDoi );

      ketQua += amLuong*sinf( tanSoNot*thoiGianTrongNot*tanSo + amLuongChinh*sinf(tanSoChinh*thoiGianTrongNot*tanSoNot ) );
      soBaoBi++;
   }
   return ketQua;
}

float tinhGiaTriNotDanDoiTanSoCap( float tanSoNot, DanDoiTanSoCap *danDoiTanSoCap, float thoiGianTrongNot, float hatBaoLau ) {

   unsigned char soLuongBaoBi = danDoiTanSoCap->soLuongBaoBi;
   unsigned char soBaoBi = 0;
   float ketQua = 0.0f;

   while( soBaoBi < soLuongBaoBi ) {
      float tanSoChinh0 = danDoiTanSoCap->tanSoChinh0[soBaoBi];            // tần số
      float tanSoChinh1 = danDoiTanSoCap->tanSoChinh1[soBaoBi];            // tần số
      float amLuongChinh0 = danDoiTanSoCap->amLuongChinh0[soBaoBi];            // tần số
      float amLuongChinh1 = danDoiTanSoCap->amLuongChinh1[soBaoBi];            // tần số
      
      BaoBi *baoBi = &(danDoiTanSoCap->baoBi[soBaoBi]);    // bao bì
      BaoBi *baoBiChinh0 = &(danDoiTanSoCap->baoBiChinh0[soBaoBi]);    // bao bì
      BaoBi *baoBiChinh1 = &(danDoiTanSoCap->baoBiChinh1[soBaoBi]);    // bao bì
      unsigned char baoBiTuongDoi = danDoiTanSoCap->baoBiTuongDoi;
      
      float amLuong = tinhAmLuongTuBaoBi( baoBi, thoiGianTrongNot, hatBaoLau, baoBiTuongDoi );
      //   float amLuongChinh = amLuongChinhDau + (amLuongChinhCuoi - amLuongChinhDau)*thoiGianTrongNot;
      float amLuong0 = amLuongChinh0*tinhAmLuongTuBaoBi( baoBiChinh0, thoiGianTrongNot, hatBaoLau, baoBiTuongDoi );
      float amLuong1 = amLuongChinh1*tinhAmLuongTuBaoBi( baoBiChinh1, thoiGianTrongNot, hatBaoLau, baoBiTuongDoi );
      
      ketQua += amLuong*sinf( amLuong0*sinf( tanSoNot*thoiGianTrongNot*tanSoChinh0 ) + amLuong1*sinf( tanSoNot*thoiGianTrongNot*tanSoChinh1 ) );
      soBaoBi++;
   }
   
   return ketQua;
}

float tinhGiaTriNotDanHuyenNaoTrang( DanHuyenNaoTrang *danHuyenNaoTrang, float thoiGianTrongNot, float hatBaoLau ) {
   

   float amLuongNot = danHuyenNaoTrang->amLuong;            // âm lượng
   BaoBi *baoBi = &(danHuyenNaoTrang->baoBi);    // bao bì
   unsigned char baoBiTuongDoi = danHuyenNaoTrang->baoBiTuongDoi;

   return rand()*tinhAmLuongTuBaoBi( baoBi, thoiGianTrongNot, hatBaoLau, baoBiTuongDoi )/ kSO_NGUYEN_TOI_DA;
}

/* Phân Tích Nốt Nhạc */
#pragma mark ---- Phân Tích Nốt Nhạc
float phanTichGiaiDieu5( Not *notNhac );

float tinhTanSoNot( Not *notNhac ) {

   if( notNhac->giaiDieu == kGIAI_DIEU_5 ) {
      return phanTichGiaiDieu5( notNhac );
   }
   else {
      printf( "SAI LẦM: Giai Điệu lạ %d\n", notNhac->giaiDieu );
   }

   return hTAN_SO_CO_BAN;
}

float phanTichGiaiDieu5( Not *notNhac ) {
   
   char doan = notNhac->doan;
   float tanSo;

   if( doan == -4 )
      tanSo = 0.0625f*hTAN_SO_CO_BAN;
   else if( doan == -3 )
      tanSo = 0.125f*hTAN_SO_CO_BAN;
   else if( doan == -2 )
      tanSo = 0.25f*hTAN_SO_CO_BAN;
   else if( doan == -1 )
      tanSo = 0.5f*hTAN_SO_CO_BAN;

   else if( doan == 1 )
      tanSo = 2.0f*hTAN_SO_CO_BAN;
   else if( doan == 2 )
      tanSo = 4.0f*hTAN_SO_CO_BAN;
   else if( doan == 3 )
      tanSo = 8.0f*hTAN_SO_CO_BAN;
   else if( doan == 4 )
      tanSo = 16.0f*hTAN_SO_CO_BAN;
   else if( doan == 5 )
      tanSo = 32.0f*hTAN_SO_CO_BAN;
   else if( doan == 6 )
      tanSo = 64.0f*hTAN_SO_CO_BAN;
   else if( doan == 7 )
      tanSo = 128.0f*hTAN_SO_CO_BAN;
   else if( doan == 8 )
      tanSo = 256.0f*hTAN_SO_CO_BAN;

   else //if( doan == 0 )
      tanSo = hTAN_SO_CO_BAN;
   
   char so = notNhac->so;
   
   if( so == 0 )
      tanSo *= 1.166667f;
   else if( so == 2 )
      tanSo *= 1.33333f;
   else if( so == 3 )
      tanSo *= 1.5f;
   else if( so == 4 )
      tanSo *= 1.666667f;
   
   return tanSo;
}


#pragma mark ---- Hát Bao Bì
/*float hatBaoBi( unsigned char loaiBaoBi, BaoBi *baoBi, float thoiHat, float thoiGianTuongDoi ) {
   
   float amLuong = 0.0f;
   
   if( loaiBaoBi == kBAO_BI__DEU )
      amLuong = tinhAmLuong_baoBiDeu( &(baoBi->baoBiDeu), thoiHat, thoiGianTuongDoi );
   else if( loaiBaoBi == kBAO_BI__MOC )
      amLuong = tinhAmLuong_baoBiMoc( &(baoBi->baoBiMoc), thoiHat, thoiGianTuongDoi );
   else
      printf( "Không biết loại bao bì %d\n", loaiBaoBi );
   
   return amLuong;
}*/

/* Kết Xuất Âm Thanh */
#pragma mark ---- Kết Xuất Âm Thanh

/*void ketXuatAmThanhNhiKenh( DuLieuAmThanh *duLieuAmThanh, Dan *dan, float tanSo, float thoiBatDauHat, float thoiDoanHat, float amLuongNot ) {
   
   // ---- dịch trong đệm
   unsigned int dichDem = duLieuAmThanh->tocDoMauVat*thoiBatDauHat*duLieuAmThanh->soLuongKenh;
   // ---- nốu thời gain sau kết thúc đệm, không cần làm gì cả
   if( dichDem > duLieuAmThanh->soLuongMauVat )
      return;

   // ---- đừng cho ra ngoài phạm vị đệm
   unsigned int soLuongMauVatTinh = duLieuAmThanh->tocDoMauVat*thoiDoanHat*duLieuAmThanh->soLuongKenh;
   
   if( dichDem + soLuongMauVatTinh > duLieuAmThanh->soLuongMauVat )
      soLuongMauVatTinh = duLieuAmThanh->soLuongMauVat - dichDem;
   
   // ---- tính dữ liệu âm thanh cho mỗi hài độ
   unsigned char chiSoHaiDo = 0;
   float buocThoiGian = 1.0f/duLieuAmThanh->tocDoMauVat;

   while( chiSoHaiDo < dan->soLuongHaiDo ) {
      // tính tần số cho hài độ này
      float haiDoTinh = tanSo*dan->mangHaiDo[chiSoHaiDo]*6.2831854f; // cho sin
      float haiDoChinhTinh = 0.0f;
      float amLuongChinh = 0.0f;
      if( (dan->loaiSong[chiSoHaiDo] == kLOAI_SONG__CHINH_AM_LUONG) || (dan->loaiSong[chiSoHaiDo] == kLOAI_SONG__CHINH_TAN_SO) ) {
         haiDoChinhTinh = tanSo*dan->tanSoChinh[chiSoHaiDo]*6.2831854f; // cho sin
         amLuongChinh = dan->amLuongChinh[chiSoHaiDo];
      }
   
      float chuKy = 1.0/(tanSo*dan->mangHaiDo[chiSoHaiDo]);  // cho tam giác, răng cưa, vuông
      float thoiGian = 0.0f;
      unsigned int chiSoMauVat = dichDem;
      unsigned char loaiSong = dan->loaiSong[chiSoHaiDo];

      while( chiSoMauVat < soLuongMauVatTinh + dichDem ) {
         float thoiGianDonViHoa = thoiGian/thoiDoanHat;  // cho được tính âm lượng từ bao bì
         // ---- tính âm lượng từ bao bì
         float amLuong = amLuongNot*hatBaoBi( dan->mangLoaiBaoBi[chiSoHaiDo], &(dan->mangBaoBi[chiSoHaiDo]), 0.5f, thoiGian );

         // ---- tính mẫu vật cho hai kênh
         if( loaiSong == kLOAI_SONG__SIN ) {
            duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*sinf( haiDoTinh*thoiGian );
            duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
         }
         else if( loaiSong == kLOAI_SONG__TAM_GIAC ) {
            float phanSo = thoiGian/chuKy;
            phanSo = phanSo - floor( phanSo );
            if( phanSo < 0.25f ) {
               duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*phanSo*4.0f;
               duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
            }
            else if( phanSo < 0.75f ) {
               duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*(1.0f - (phanSo - 0.25f)*4.0f);
               duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
            }
            else {
               duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*(-1.0f + (phanSo - 0.75f)*4.0f);
               duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
            }
         }
         else if( loaiSong == kLOAI_SONG__RANG_CUA_XUONG ) {
            float phanSo = thoiGian/chuKy;
            phanSo = phanSo - floor( phanSo );
            duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*(1.0f - phanSo*2.0f);
            duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
         }
         else if( loaiSong == kLOAI_SONG__RANG_CUA_LEN ) {
            float phanSo = thoiGian/chuKy;
            phanSo = phanSo - floor( phanSo );
            duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*(-1.0f + phanSo*2.0f);
            duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
         }
         else if( loaiSong == kLOAI_SONG__VUONG ) {
            float phanSo = thoiGian/chuKy;
            phanSo = phanSo - floor( phanSo );
            if( phanSo < dan->phanSoCao ) {
               duLieuAmThanh->duLieu[chiSoMauVat] += amLuong;
               duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
            }
            else {
               duLieuAmThanh->duLieu[chiSoMauVat] -= amLuong;
               duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
            }
         }
         else if( loaiSong == kLOAI_SONG__HUYEN_NAO_TRANG ) {
            duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*rand()/kSO_NGUYEN_TOI_DA;
            duLieuAmThanh->duLieu[chiSoMauVat+1] += amLuong*rand()/kSO_NGUYEN_TOI_DA;
         }
         else if( loaiSong == kLOAI_SONG__CHINH_AM_LUONG ) {
            duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*(1.0f + amLuongChinh*sinf(haiDoChinhTinh*thoiGian))*sinf(haiDoTinh*thoiGian);
            duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
         }
         else if( loaiSong == kLOAI_SONG__CHINH_TAN_SO ) {
            duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*sinf(haiDoTinh*thoiGian + amLuongChinh*sinf(haiDoChinhTinh*thoiGian) );
            duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
         }
         else if( loaiSong == kLOAI_SONG__NHAN_TAN_SO ) {
            printf( "chiSoHaiDo %d amLuong %5.3f\n", chiSoHaiDo, amLuong );
            duLieuAmThanh->duLieu[chiSoMauVat] += amLuong*sinf(haiDoTinh*thoiGian)*sinf(haiDoChinhTinh*thoiGian + 0.5f);
            duLieuAmThanh->duLieu[chiSoMauVat+1] = duLieuAmThanh->duLieu[chiSoMauVat];
    
         }
         // ---- mẫu vật tiếp
         thoiGian += buocThoiGian;
   
         chiSoMauVat += 2;
      }
      chiSoHaiDo++;
   }

} */

#pragma mark ---- Đơn Vị Hóa
void donViHoaMauVatNhiKenh( DuLieuAmThanh *duLieuAmThanh ) {

   float mauVatLonNhat = 0.0f;
   
   // ---- tìm mẫu vật có âm lượng cao nhất trong tất cả kênh
//   unsigned short chiSoKenh = 0;  // không giống chỉ số mẫu vật vì ố lượng kênh
//   unsigned short soLuongKenh = duLieuAmThanh->soLuongKenh;

   unsigned int chiSoMauVat = 0;
   unsigned int soLuongMauVat = duLieuAmThanh->soLuongMauVat;

 //  while( chiSoKenh < soLuongKenh ) {
      while( chiSoMauVat < duLieuAmThanh->soLuongMauVat ) {
         // ---- lấy độ lớn mẫu vật
         float doLonMauVat = duLieuAmThanh->mangMauVatKenh0[chiSoMauVat];
         if( doLonMauVat < 0.0f )
            doLonMauVat = -doLonMauVat;
         // ---- xem nếu độLớnMẫuVật lớn hơn mẫuVậtLớnNhất, nếu có giữ nó
         if( doLonMauVat > mauVatLonNhat )
            mauVatLonNhat = doLonMauVat;
         
         doLonMauVat = duLieuAmThanh->mangMauVatKenh1[chiSoMauVat];
         if( doLonMauVat < 0.0f )
            doLonMauVat = -doLonMauVat;
         // ---- xem nếu độLớnMẫuVật lớn hơn mẫuVậtLớnNhất, nếu có giữ nó
         if( doLonMauVat > mauVatLonNhat )
            mauVatLonNhat = doLonMauVat;
         
         // ---- mẫu vật tiếp
         chiSoMauVat++;
      }
//      chiSoKenh++;
//   }
   
   // ---- don vi hoa hết kênh
  // ---- CHỈ LÀM NẾU mẫuVậtLớnNhất khác 0
   if( (mauVatLonNhat != 0.0f) && (mauVatLonNhat > 1.0f) ) {
      chiSoMauVat = 0;
      while( chiSoMauVat < duLieuAmThanh->soLuongMauVat ) {
         duLieuAmThanh->mangMauVatKenh0[chiSoMauVat] /= mauVatLonNhat;
         duLieuAmThanh->mangMauVatKenh1[chiSoMauVat] /= mauVatLonNhat;
         chiSoMauVat++;
      }
   }
}


#pragma mark ---- Bao Bì Đều
//      |   |   |            |   |
//      |   |   +------------+   |
//      |   | / |            | \ |
//      |   |/  |            |  \|
//      +---+---+------------+---+
//    0,0  B    G            K   H
//         ắ    i            ế   á
//         t    ữ            t   t
//         Đ    a            T   B
//      |<->| thờiBắtĐầu
//      |<------>| thờiGiữa
//      |               |<->| thời Kết thúc
//      |<----------------->| hát bao lâu
//
//      |----> thời gian tương đối

// Giá trị nên 0 ≤ giữa < bắt đầu ≤ 1

// mỗi đoàn của bao bì bị đơn vị hóa;
// thời giữa là thời bắt đầu khúc giữa, đơn vị giây - không đơn vị hóa
// thời giữa là thời bắt đầu khúc kết thúc, đơn vị giây - không đơn vị hóa
float tinhAmLuong_baoBiDeu( float hatBaoLau, float thoiGianTuongDoi, float batDau, float thoiGiua, float thoiKetThuc ) {
   
   Bezier congBatDau;   // cong cho khúc bắt đầu
   Bezier congKetThuc;  // cong cho khúc kết thúc

   // ---- khúc bắt đầu
   congBatDau.diem0.x = 0.0000f;  congBatDau.diem0.y = 0.0000f;
   congBatDau.diem1.x = 0.0000f;  congBatDau.diem1.y = 0.5000f;
   congBatDau.diem2.x = 0.5000f;  congBatDau.diem2.y = 1.0000f;
   congBatDau.diem3.x = 1.0000f;  congBatDau.diem3.y = 1.0000f;
   
   // ---- khúc giữa đều gang = 1, không cần cong
   
   // ---- khúc kết thúc
   congKetThuc.diem0.x = 0.0000f;  congKetThuc.diem0.y = 1.0000f;
   congKetThuc.diem1.x = 0.5000f;  congKetThuc.diem1.y = 1.0000f;
   congKetThuc.diem2.x = 1.0000f;  congKetThuc.diem2.y = 0.5000f;
   congKetThuc.diem3.x = 1.0000f;  congKetThuc.diem3.y = 0.0000f;
   
   float amLuong = 0.0f;

   if( thoiGianTuongDoi < batDau ) {
      amLuong = 0.0f;
   }
   else if( thoiGianTuongDoi < thoiGiua ) {  // khúc bắt đầu
      // ---- đơn vị hóa
      float x = (thoiGianTuongDoi - batDau)/(thoiGiua - batDau);
      float thamSo = timThamSoChoX( &congBatDau, x );
      amLuong = tinhViTriBezierY( &congBatDau, thamSo );
   }
   else if( thoiGianTuongDoi < hatBaoLau - thoiKetThuc ) {  // khúc giữa
      amLuong = 1.0f; // ---- âm lượng không đổi
   }
   else if( thoiGianTuongDoi < hatBaoLau ) {  // khúc kết thúc
      float mauSo = hatBaoLau - thoiKetThuc;
//      printf( "mauSo %5.3f\n", mauSo );

      float x = (thoiGianTuongDoi - mauSo)/thoiKetThuc;
      float thamSo = timThamSoChoX( &congKetThuc, x );
      amLuong = tinhViTriBezierY( &congKetThuc, thamSo );
   }

   if( thoiGianTuongDoi > hatBaoLau )
      amLuong = 0.0f;

   return amLuong;
}


#pragma mark ---- Bao Bì Móc 0
//      |    |\
//      |    | -- \
//      |    |     ----\
//      |    |          --------\
//      +----+-----------------------+
//      |--->| thờiBắtĐầu
//      |--------------------------->| hát bao lâu
//
//      |----> thời gian tương đối

float tinhAmLuong_baoBiMoc0( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau ) {

   Bezier cong;
   
   // ---- các điểm này cho cong Bezier làm đường bậc một
   cong.diem0.x = 0.0000f;  cong.diem0.y = 1.0000f;
   cong.diem1.x = 0.0000f;  cong.diem1.y = 0.5000f;
   cong.diem2.x = 0.5000f;  cong.diem2.y = 0.0000f;
   cong.diem3.x = 1.0000f;  cong.diem3.y = 0.0000f;

   float amLuong = 0.0f;
   
   if( thoiGianTuongDoi < thoiBatDau ) {
      amLuong = 0.0f;
   }
   else if( thoiGianTuongDoi < hatBaoLau ) {
      float x = (thoiGianTuongDoi - thoiBatDau)/(hatBaoLau - thoiBatDau);
      float thamSo = timThamSoChoX( &cong, x );
      amLuong = tinhViTriBezierY( &cong, thamSo );
   }
   
   return amLuong;
}


#pragma mark ---- Bao Bì Móc 1
//      |    |     /---+-         |
//      |    |   --    | \        |
//      |    | /       |   --\    |
//      |    |/        |      ---\|
//      +----+---------+----------+
//    0,0                    1,0
//
//      |--->| thờiBắtĐầu
//      |------------->| thờiGiữa
//      |------------------------>| hát bao lâu
//
//      |----> thời gian tương đối

// thời kết thúc là thời bắt đầu khúc kết thúc, đơn vị giây - không đơn vị hóa

float tinhAmLuong_baoBiMoc1( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau, float thoiGiua ) {
   
   Bezier congBatDau;  // cong cho khúc bắt đầu
   Bezier congKetThuc; // cong cho khúc kết thúc
   
   congBatDau.diem0.x = 0.0000f;  congBatDau.diem0.y = 0.0000f;
   congBatDau.diem1.x = 0.0000f;  congBatDau.diem1.y = 0.5000f;
   congBatDau.diem2.x = 0.5000f;  congBatDau.diem2.y = 1.0000f;
   congBatDau.diem3.x = 1.0000f;  congBatDau.diem3.y = 1.0000f;
   
   congKetThuc.diem0.x = 0.0000f;  congKetThuc.diem0.y = 1.0000f;
   congKetThuc.diem1.x = 0.2000f;  congKetThuc.diem1.y = 1.0000f;
   congKetThuc.diem2.x = 0.5000f;  congKetThuc.diem2.y = 0.0000f;
   congKetThuc.diem3.x = 1.0000f;  congKetThuc.diem3.y = 0.0000f;
   
   float amLuong = 0.0f;

   if( thoiGianTuongDoi < thoiBatDau ) {
      amLuong = 0.0f;
   }
   else if( thoiGianTuongDoi < thoiGiua ) {
      float x = (thoiGianTuongDoi - thoiBatDau)/(thoiGiua - thoiBatDau);
      float thamSo = timThamSoChoX( &congBatDau, x );
      amLuong = tinhViTriBezierY( &congBatDau, thamSo );
   }
   else {
      float x = (thoiGianTuongDoi - thoiGiua)/(hatBaoLau - thoiGiua);
      float thamSo = timThamSoChoX( &congBatDau, x );
      amLuong = tinhViTriBezierY( &congKetThuc, thamSo );
   }
   return amLuong;
}


#pragma mark ---- Bao Bì Đòi
//      |    |        --+--        |
//      |    |       /  |  \       |
//      |    |    /--   |   --\    |
//      |    |/---      |      ---\|
//      +----+----------+----------+
//    0,0
//      |--->| thờiBắtĐầu
//      |-------------->| thờiGiữa
//      |------------------------->| hát bao lâu
//
//      |----> thời gian tương đối
// thời kết thúc là thời bắt đầu khúc kết thúc, đơn vị giây - không đơn vị hóa

float tinhAmLuong_baoBiDoi( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau, float thoiGiua ) {

   Bezier congBatDau;
   Bezier congKetThuc;
   
   // ---- các điểm này cho cong Bezier làm đường bậc một
   congBatDau.diem0.x = 0.0000f;  congBatDau.diem0.y = 0.0000f;
   congBatDau.diem1.x = 0.5000f;  congBatDau.diem1.y = 0.0000f;
   congBatDau.diem2.x = 0.5000f;  congBatDau.diem2.y = 1.0000f;
   congBatDau.diem3.x = 1.0000f;  congBatDau.diem3.y = 1.0000f;
   
   congKetThuc.diem0.x = 0.0000f;  congKetThuc.diem0.y = 1.0000f;
   congKetThuc.diem1.x = 0.5000f;  congKetThuc.diem1.y = 1.0000f;
   congKetThuc.diem2.x = 0.5000f;  congKetThuc.diem2.y = 0.0000f;
   congKetThuc.diem3.x = 1.0000f;  congKetThuc.diem3.y = 0.0000f;
   
   float amLuong = 0.0f;
   
   if( thoiGianTuongDoi < thoiBatDau ) {
      amLuong = 0.0f;
   }
   else if( thoiGianTuongDoi < thoiGiua ) {
      float x = (thoiGianTuongDoi - thoiBatDau)/(thoiGiua - thoiBatDau);
      float thamSo = timThamSoChoX( &congBatDau, x );
      amLuong = tinhViTriBezierY( &congBatDau, thamSo );
//      printf( "else if amLuong %5.3f  thoiGianTuongDoi %5.3f  hatBaoLAu %5.3f, thoiGiua %5.3f  x %5.3f  thamSo %5.3f\n", amLuong, thoiGianTuongDoi, hatBaoLau, thoiGiua, x, thamSo );
   }
   else {
      float x = (thoiGianTuongDoi - thoiGiua)/(hatBaoLau - thoiGiua);
      float thamSo = timThamSoChoX( &congKetThuc, x );
      amLuong = tinhViTriBezierY( &congKetThuc, thamSo );
//      printf( "amLuong %5.3f  thoiGianTuongDoi %5.3f  hatBaoLAu %5.3f, thoiGiua %5.3f  x %5.3f  thamSo %5.3f\n", amLuong, thoiGianTuongDoi, hatBaoLau, thoiGiua, x, thamSo );

//      printf( "else\n" );
//      exit(0);
   }

   return amLuong;
}


#pragma mark ---- Bao Bì Dóc
//      |   |
//      |   +--------\          |
//      |   |         ----\     |
//      |   |              --\  |
//      |   |                 -\|
//      +---+-------------------+
//    0,0
//      |-->| thờiBắtĐầu
//      |---------------------->| hát bao lâu
//
//      |----------> thời gian tương đối
// thời kết thúc là thời bắt đầu khúc kết thúc, đơn vị giây - không đơn vị hóa

float tinhAmLuong_baoBiDoc( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau ) {
   
   Bezier cong;
   
   // ---- các điểm này cho cong Bezier làm đường bậc một
   cong.diem0.x = 0.0000f;  cong.diem0.y = 1.0000f;
   cong.diem1.x = 0.7000f;  cong.diem1.y = 1.0000f;
   cong.diem2.x = 1.0000f;  cong.diem2.y = 0.7000f;
   cong.diem3.x = 1.0000f;  cong.diem3.y = 0.0000f;
   
   float amLuong = 0.0f;
   
   if( thoiGianTuongDoi < thoiBatDau ) {
      amLuong = 0.0f;
   }
   else {
      float x = (thoiGianTuongDoi - thoiBatDau)/(hatBaoLau - thoiBatDau);
      float thamSo = timThamSoChoX( &cong, x );
      amLuong = tinhViTriBezierY( &cong, thamSo );
   }
   
   return amLuong;
}

#pragma mark ---- Bao Bì Chuẩn
//      |         |   |      |
//      |         +   |      |
//      |        /| \ |      |
//      |       / |   +------+
//      |      /  |   |      | \
//      |     /   |   |      |   \
//      +----+---+------+----+----+
//      |<-->| thờiBắtĐầu
//      |<------->| thờiHạ
//      |<----------->| thờiGiữa
//      |                    |<-->| thời Kết thúc
//      |<----------------------->| hát bao lâu
//
//      |----> thời gian tương đối

float tinhAmLuong_baoBiChuan( float hatBaoLau, float thoiGianTuongDoi, float thoiBatDau, float thoiHa, float thoiGiua, float thoiKetThuc ) {
   

   float amLuongGiua = 0.8f;

   float amLuong = 0.0f;
   
   if( thoiGianTuongDoi < thoiBatDau ) {
      amLuong = 0.0f;
   }
   else if( thoiGianTuongDoi < thoiHa ) {
      amLuong = (thoiGianTuongDoi - thoiBatDau)/(thoiHa - thoiBatDau);
   }
   else if( thoiGianTuongDoi < thoiGiua ) {
      amLuong = 1.0f - (1.0f - amLuongGiua)*(thoiGianTuongDoi - thoiHa)/(thoiGiua - thoiHa);
   }
   else if( thoiGianTuongDoi < (hatBaoLau - thoiKetThuc) ) {
      amLuong = amLuongGiua;
   }
   else {
      amLuong = amLuongGiua - amLuongGiua*(thoiGianTuongDoi - (hatBaoLau - thoiKetThuc))/thoiKetThuc;
   }
   
   return amLuong;
}


#pragma mark ---- LƯU ÂM THANH
void luuAmThanh( char *tenTep, DuLieuAmThanh *duLieuAmThanh, unsigned char kieuDuLieu ) {

   // ---- mở tệp
   FILE *tep  = fopen(  tenTep, "wb" );
   
   if( tep ) {
      // ---- "RIFF"
      fprintf( tep, "RIFF" );
      
      // ---- bề dài âm thanh tệp - 8
      unsigned int beDai = 32;
      if( kieuDuLieu == hKIEU_PCM_16 )
         beDai += ((duLieuAmThanh->soLuongMauVat)*(duLieuAmThanh->soLuongKenh)) << 1; // 2 byte
      else if( kieuDuLieu == hKIEU_FLOAT )
         beDai += ((duLieuAmThanh->soLuongMauVat)*(duLieuAmThanh->soLuongKenh)) << 2; // 4 byte
      printf( "beDai %d   duLieuAmThanh->soLuongMauVat %d\n", beDai, duLieuAmThanh->soLuongMauVat );

      // ---- lưu bề dài
      fputc( (beDai) & 0xff, tep );
      fputc( (beDai >> 8) & 0xff, tep );
      fputc( (beDai >> 16) & 0xff, tep );
      fputc( (beDai >> 24) & 0xff, tep );
      
      // ---- "WAVE"
      fprintf( tep, "WAVE" );
      
      // ---- lưu ầầu tệp
      luuThongTinDau( tep, duLieuAmThanh, kieuDuLieu );
      
      // ---- lưu dữ liệu tệp
      if( kieuDuLieu == hKIEU_PCM_16 )
         luuDuLieuSoNguyen16bit_nhiKenh( tep, duLieuAmThanh );
      else if( kieuDuLieu == hKIEU_FLOAT )
         luuDuLieuSoThat32bit_nhiKenh( tep, duLieuAmThanh );
      
      // ---- đống tệp
      fclose( tep );
   }
   else {
      printf( "Vấn đề lưu tệp %s\n", tenTep );
   }
}

void luuThongTinDau( FILE *tep, DuLieuAmThanh *duLieuAmThanh, unsigned short kieuDuLieu ) {

   // ---- tên cho đầu "fmt "
   fprintf( tep, "fmt " );

   // ---- bề dài
   fputc( 0x10, tep );  // 16 byte
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );

   // ---- phương pháp mã hóa
   fputc( kieuDuLieu & 0xff, tep );
   fputc( (kieuDuLieu  >> 8) & 0xff, tep );

   // ---- sô lượng kênh
   fputc( (duLieuAmThanh->soLuongKenh) & 0xff, tep );
   fputc( (duLieuAmThanh->soLuongKenh >> 8) & 0xff, tep );

   // ---- tốc độ mẫu vật (Hz)
   fputc( duLieuAmThanh->tocDoMauVat & 0xff, tep );
   fputc( (duLieuAmThanh->tocDoMauVat >> 8) & 0xff, tep );
   fputc( (duLieuAmThanh->tocDoMauVat >> 16) & 0xff, tep );
   fputc( (duLieuAmThanh->tocDoMauVat >> 24) & 0xff, tep );
   
   // ---- số lượng byte/giây
   unsigned int soLuongByteGiay = duLieuAmThanh->tocDoMauVat;
   if( kieuDuLieu == hKIEU_PCM_16 )
      soLuongByteGiay *= 2*duLieuAmThanh->soLuongKenh;
   else if( kieuDuLieu == hKIEU_FLOAT )
      soLuongByteGiay *= 4*duLieuAmThanh->soLuongKenh;

   fputc( soLuongByteGiay & 0xff, tep );
   fputc( (soLuongByteGiay >> 8) & 0xff, tep );
   fputc( (soLuongByteGiay >> 16) & 0xff, tep );
   fputc( (soLuongByteGiay >> 24) & 0xff, tep );

   // ---- byte cho một cục (nên = số lượng mẫu vật * số lượng kênh)
  // ---- số lượng byte mẫu vật
   unsigned short byteKhungMauVat = 0;
   unsigned char soLuongBitMauVat = 0;
   if( kieuDuLieu == hKIEU_PCM_16 ) {
       byteKhungMauVat = duLieuAmThanh->soLuongKenh * 2;
       soLuongBitMauVat = 16;
   }
   else if( kieuDuLieu == hKIEU_PCM_16 ) {
      byteKhungMauVat = duLieuAmThanh->soLuongKenh * 4;
      soLuongBitMauVat = 32;
   }
   else 
      printf( "Sai lầm: kiểu dữ liệu lạ: %x\n", kieuDuLieu );

   fputc( (byteKhungMauVat) & 0xff, tep );
   fputc( (byteKhungMauVat >> 8) & 0xff, tep );
   fputc( soLuongBitMauVat, tep );
   fputc( 0x00, tep );
}
   
void luuDuLieuSoNguyen16bit_nhiKenh( FILE *tep, DuLieuAmThanh *duLieuAmThanh ) {

   fprintf( tep, "data" );
   
   // ---- tính số lượng mẫu vật
   unsigned int soLuongByte = duLieuAmThanh->soLuongMauVat << 2;  // nhị kênh, 2 byte
   
   fputc( soLuongByte & 0xff, tep );
   fputc( (soLuongByte >> 8) & 0xff, tep );
   fputc( (soLuongByte >> 16) & 0xff, tep );
   fputc( (soLuongByte >> 24) & 0xff, tep );

   unsigned int soLuongMauVat = duLieuAmThanh->soLuongMauVat;
   unsigned int chiSoMauVat = 0;
   while( chiSoMauVat < soLuongMauVat ) {

      short duLieuShort = (short)(duLieuAmThanh->mangMauVatKenh0[chiSoMauVat]*32767.0f);
      fputc( duLieuShort & 0xff, tep );
      fputc( (duLieuShort >> 8) & 0xff, tep );
      duLieuShort = (short)(duLieuAmThanh->mangMauVatKenh1[chiSoMauVat]*32767.0f);
      fputc( duLieuShort & 0xff, tep );
      fputc( (duLieuShort >> 8) & 0xff, tep );

      chiSoMauVat++;
   }

}

void luuDuLieuSoThat32bit_nhiKenh( FILE *tep, DuLieuAmThanh *duLieuAmThanh ) {

   fprintf( tep, "data" );

   // ---- tính số lượng mẫu vật
   unsigned int soLuongByte = duLieuAmThanh->soLuongMauVat*duLieuAmThanh->soLuongKenh << 3;  // nhị kênh, 4 byte

   fputc( soLuongByte & 0xff, tep );
   fputc( (soLuongByte >> 8) & 0xff, tep );
   fputc( (soLuongByte >> 16) & 0xff, tep );
   fputc( (soLuongByte >> 24) & 0xff, tep );
   
   unsigned int soLuongMauVat = duLieuAmThanh->soLuongMauVat;
   unsigned int chiSoMauVat = 0;
   while( chiSoMauVat < soLuongMauVat ) {
      union {
         float f;
         unsigned int i;
      } ui;
      
      ui.f = duLieuAmThanh->mangMauVatKenh0[chiSoMauVat];
      fputc( ui.i & 0xff, tep );
      fputc( (ui.i >> 8) & 0xff, tep );
      fputc( (ui.i >> 16) & 0xff, tep );
      fputc( (ui.i >> 24) & 0xff, tep );
      
      ui.f = duLieuAmThanh->mangMauVatKenh1[chiSoMauVat];
      fputc( ui.i & 0xff, tep );
      fputc( (ui.i >> 8) & 0xff, tep );
      fputc( (ui.i >> 16) & 0xff, tep );
      fputc( (ui.i >> 24) & 0xff, tep );

      chiSoMauVat++;
   }
}


#pragma mark ---- Bezier
float tinhViTriBezierX( Bezier *bezier, float thamSo ) {
   
   // ---- mức 0, cần tính 3 điểm
   float muc0[3];
   muc0[0] = (1.0f - thamSo)*bezier->diem0.x + thamSo*bezier->diem1.x;
   muc0[1] = (1.0f - thamSo)*bezier->diem1.x + thamSo*bezier->diem2.x;
   muc0[2] = (1.0f - thamSo)*bezier->diem2.x + thamSo*bezier->diem3.x;
   
   // ---- mức 1, cần tính 2 điểm
   float muc1[2];
   muc1[0] = (1.0f - thamSo)*muc0[0] + thamSo*muc0[1];
   muc1[1] = (1.0f - thamSo)*muc0[1] + thamSo*muc0[2];
   
   // ---- tính giá trị
   float giaTri = (1.0f - thamSo)*muc1[0] + thamSo*muc1[1];
   
   return giaTri;
}

float tinhViTriBezierY( Bezier *bezier, float thamSo ) {
   
   // ---- mức 0, cần tính 3 điểm
   float muc0[3];
   muc0[0] = (1.0f - thamSo)*bezier->diem0.y + thamSo*bezier->diem1.y;
   muc0[1] = (1.0f - thamSo)*bezier->diem1.y + thamSo*bezier->diem2.y;
   muc0[2] = (1.0f - thamSo)*bezier->diem2.y + thamSo*bezier->diem3.y;
   
   // ---- mức 1, cần tính 2 điểm
   float muc1[2];
   muc1[0] = (1.0f - thamSo)*muc0[0] + thamSo*muc0[1];
   muc1[1] = (1.0f - thamSo)*muc0[1] + thamSo*muc0[2];
   
   // ---- tính giá trị
   float giaTri = (1.0f - thamSo)*muc1[0] + thamSo*muc1[1];
   
   return giaTri;
}

#define hGIOI_HAN_XA 0.0001f
float timThamSoChoX( Bezier *bezier, float x ) {
   
   if( x >= 1.0f )
      return tinhViTriBezierX( bezier, 1.0f );
   else if( x <= 0.0f )
      return tinhViTriBezierX( bezier, 0.0f );

   float thamSoThap = 0.0f;
   float thamSoGiua = 0.5f;
   float thamSoCao = 1.0f;
   float thamSo = 0.0f;
      
   float giaTriThap = tinhViTriBezierX( bezier, thamSoThap );
   float giaTriCao = tinhViTriBezierX( bezier, thamSoCao );
//   printf( "giaTriThap %5.3f  cao %5.3f  x %5.3f\n", giaTriThap, giaTriCao, x );
   unsigned char xong = hSAI;

   // ---- coi thử được xong sớm
   float chenhLech = x - giaTriThap;
   if( chenhLech < 0.0f )
      chenhLech = -chenhLech;
   if( chenhLech < hGIOI_HAN_XA ) {
      xong = hDUNG;
      thamSo = thamSoThap;
   }
   
   chenhLech = x - giaTriCao;
   if( chenhLech < 0.0f )
      chenhLech = -chenhLech;
   if( chenhLech < hGIOI_HAN_XA ) {
      xong = hDUNG;
      thamSo = thamSoCao;
   }
   unsigned int soLuongLapLai = 0;
   
   while( !xong ) {
      thamSoGiua = (thamSoThap + thamSoCao) * 0.5f;
      float giaTriGiua = tinhViTriBezierX( bezier, thamSoGiua );

      chenhLech = x - giaTriGiua;
      if( chenhLech < 0.0f )
         chenhLech = -chenhLech;
      if( chenhLech < hGIOI_HAN_XA ) {
         xong = hDUNG;
         thamSo = thamSoGiua;
      }
//      printf( "%d giaTriGiua %5.3f  thap %5.3f  cao %5.3f   thamSoGiua %5.3f  thap %5.3f  cao %5.3f  chenhLech %5.3f\n", soLuongLapLai, giaTriGiua, giaTriThap, giaTriCao, thamSoGiua, thamSoThap, thamSoCao, chenhLech );
      // ---- chỉ cần làm nếu chưa xong
      if( !xong ) {
         if( x > giaTriGiua ) {
            thamSoThap = thamSoGiua;
//            giaTriThap = giaTriGiua;
         }
         else {
            thamSoCao = thamSoGiua;
//            giaTriCao = giaTriGiua;
         }
         soLuongLapLai++;
         if( soLuongLapLai > 20 ) {
            printf( "timThamSoChoX: sai lầm thứ tự x trong cong Bezier: soLuongLapLai %d  x %5.3f\n", soLuongLapLai, x );
            //         exit(0);
            return giaTriGiua;
         }
      }
   }
   
   return thamSo;
}


float tinhAmLuongTuBaoBi( BaoBi *baoBi, float thoiGianTrongNot, float thoiHat, unsigned char baoBiTuongDoi ) {
   
   unsigned char loai = baoBi->loai;

   float giaTri = 0.0f;
   if( loai == kBAO_BI__DEU ) {
      if( baoBiTuongDoi )
         giaTri = tinhAmLuong_baoBiDeu( thoiHat, thoiGianTrongNot, (baoBi->batDau)*thoiHat, baoBi->giua*thoiHat, baoBi->ketThuc*thoiHat );
      else
         giaTri = tinhAmLuong_baoBiDeu( thoiHat, thoiGianTrongNot, baoBi->batDau, baoBi->giua, baoBi->ketThuc );
   }
   else if( loai == kBAO_BI__MOC0 ) {
      giaTri = tinhAmLuong_baoBiMoc0( thoiHat, thoiGianTrongNot, baoBi->batDau );
   }
   else if( loai == kBAO_BI__MOC1 ) {
      giaTri = tinhAmLuong_baoBiMoc1( thoiHat, thoiGianTrongNot, baoBi->batDau, baoBi->giua );
   }
   else if( loai == kBAO_BI__DOI ) {
      giaTri = tinhAmLuong_baoBiDoi( thoiHat, thoiGianTrongNot, baoBi->batDau, baoBi->giua );
   }
   else { // kBAO_BI__KHONG
      giaTri = 1.0f;
   }
//   printf( "thoiGianDV %5.3f  giaTri %5.3f   thoiHat %5.3f\n", thoiGianTrongNot, giaTri, thoiHat );
   // ---- tính âm lượng
   return giaTri*baoBi->amLuong;
}


#pragma mark ---- Bỏ Âm Nhạc Nhi Kênh
void boAmTruong_nhiKenh( AmNhacTruong *amNhacTruong ) {
   // ---- bỏ kênh
   free( amNhacTruong->duLieuAmThanh.mangMauVatKenh0 );
   free( amNhacTruong->duLieuAmThanh.mangMauVatKenh1 );
   
   // ---- bỏ làn
   free( amNhacTruong->danhSachLan );
}

#pragma mark ---- Các Đàn
//  +
//  |
//  |  thời hát bao bì thường
//  |------>|
//  |       |----->| thời hát bao bì kết thúc
//  |
//  +-------+------+
//
//  |------------->|  thời hát toàn nốt
//
//  +
//  |
//  |  thời hát bao bì thường
//  |-------------------->|
//  |                     |----->| thời hát bao bì kết thúc
//  |
//  +---------------------+------+
//
//  |--------------------------->|  thời hát toàn nốt

// tanSo 1.00, 1.96, 3.92, 3.88
// amLuong 1.00, 0.95, 0.8, 0.5
DanSong datDan_danSongNgauNhien() {
   
   DanSong danSong;
   danSong.soLuongTanSo = 1 + (rand() & 7);  // số lượng tần số

   unsigned short soTanSo = 0;
   while( soTanSo < danSong.soLuongTanSo ) {
      if( soTanSo == 0 ) {
           danSong.mangTanSo[soTanSo] = 1.0f;
      }
      else {
         danSong.mangTanSo[soTanSo] = (float)(soTanSo + 1) + ((rand() % 1000) * 0.0005) - 0.05;

      }
      danSong.mangBaoBi[soTanSo].amLuong = 0.1f + (rand() % 900) * 0.001f;
      danSong.mangBaoBi[soTanSo].loai = kBAO_BI__DOI;
      danSong.mangBaoBi[soTanSo].batDau = (rand() % 1000) * 0.001f;
      danSong.mangBaoBi[soTanSo].giua = 1.0f + (rand() % 1000)*0.002f;
      danSong.mangBaoBi[soTanSo].ketThuc = (rand() % 1000) * 0.001f;

      printf( "%d -- tanSo %5.3f  amLuong %5.3f  batDat %5.3f  giua %5.3f  ketThuc %5.3f\n", soTanSo, danSong.mangTanSo[soTanSo],
             danSong.mangBaoBi[soTanSo].amLuong, danSong.mangBaoBi[soTanSo].batDau, danSong.mangBaoBi[soTanSo].giua,
             danSong.mangBaoBi[soTanSo].ketThuc );
      soTanSo++;
   }
   // ---- đặt các tần số và bao bì
   
   return danSong;
}


DanSong datDan_danDay0() {
   
   DanSong danSong;
   danSong.soLuongTanSo = 4;  // số lượng tần số
   danSong.baoBiTuongDoi = 1;
   
   danSong.mangTanSo[0] = 1.0f;
   danSong.mangBaoBi[0].amLuong = 0.3f;
   danSong.mangBaoBi[0].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[0].batDau = 0.0f;
   danSong.mangBaoBi[0].ketThuc = 0.0f;
   
   danSong.mangTanSo[1] = 2.0f;
   danSong.mangBaoBi[1].amLuong = 0.48f;
   danSong.mangBaoBi[1].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[1].batDau = 0.0f;
   danSong.mangBaoBi[1].ketThuc = 0.2f;
   
   danSong.mangTanSo[2] = 3.0f;
   danSong.mangBaoBi[2].amLuong = 0.6f;
   danSong.mangBaoBi[2].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[2].batDau = 0.0f;
   danSong.mangBaoBi[2].ketThuc = 0.6f;
   
   danSong.mangTanSo[3] = 4.0f;
   danSong.mangBaoBi[3].amLuong = 1.0f;
   danSong.mangBaoBi[3].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[3].batDau = 0.0f;
   danSong.mangBaoBi[3].ketThuc = 0.8f;
   
   return danSong;
}


DanSong datDan_danDay1() {
   
   DanSong danSong;
   danSong.soLuongTanSo = 3;  // số lượng tần số
   danSong.baoBiTuongDoi = 1;
   
   danSong.mangTanSo[0] = 1.0f;
   danSong.mangBaoBi[0].amLuong = 1.0f;
   danSong.mangBaoBi[0].loai = kBAO_BI__DOI;
   danSong.mangBaoBi[0].batDau = 0.0f;
   danSong.mangBaoBi[0].giua = 0.35f;
   danSong.mangBaoBi[0].ketThuc = 0.0f;
   
   danSong.mangTanSo[1] = 2.0f;
   danSong.mangBaoBi[1].amLuong = 0.27f;
   danSong.mangBaoBi[1].loai = kBAO_BI__DOI;
   danSong.mangBaoBi[1].batDau = 0.0f;
   danSong.mangBaoBi[1].giua = 0.30f;
   danSong.mangBaoBi[1].ketThuc = 0.25f;
   
   danSong.mangTanSo[2] = 3.0f;
   danSong.mangBaoBi[2].amLuong = 0.1f;
   danSong.mangBaoBi[2].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[2].batDau = 0.0f;
   danSong.mangBaoBi[2].giua = 0.35f;
   danSong.mangBaoBi[2].ketThuc = 0.5f;
   
   return danSong;
}


DanSong datDan_danSong0() {
   
   DanSong danSong;
   danSong.soLuongTanSo = 4;  // số lượng tần số
   danSong.baoBiTuongDoi = 1;
   
   danSong.mangTanSo[0] = 1.0f;
   danSong.mangBaoBi[0].amLuong = 0.3f;
   danSong.mangBaoBi[0].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[0].batDau = 0.0f;
   danSong.mangBaoBi[0].ketThuc = 0.0f;
   
   danSong.mangTanSo[1] = 2.0f;
   danSong.mangBaoBi[1].amLuong = 0.48f;
   danSong.mangBaoBi[1].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[1].batDau = 0.0f;
   danSong.mangBaoBi[1].ketThuc = 0.2f;
   
   danSong.mangTanSo[2] = 3.0f;
   danSong.mangBaoBi[2].amLuong = 0.6f;
   danSong.mangBaoBi[2].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[2].batDau = 0.0f;
   danSong.mangBaoBi[2].ketThuc = 0.6f;
   
   danSong.mangTanSo[3] = 4.0f;
   danSong.mangBaoBi[3].amLuong = 1.0f;
   danSong.mangBaoBi[3].loai = kBAO_BI__MOC0;
   danSong.mangBaoBi[3].batDau = 0.0f;
   danSong.mangBaoBi[3].ketThuc = 0.8f;
   
   return danSong;
}


DanSong datDan_chuongSat() {
   
   float mangTanSo[10] = {1.0f, 2.0f, 2.6f, 2.9f, 3.0f, 3.2f, 4.2f, 5.6f, 6.6f, 8.2f,};
   float mangAmLuong[10] = {1.0f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f, .1f, 0.1f, 0.1f, 0.1f};
   DanSong danSong;
   danSong.soLuongTanSo = 10;  // số lượng tần số
   danSong.baoBiTuongDoi = 1;

   unsigned char soTanSo = 0;
   while( soTanSo < 10 ) {
   // ---- đặt các tần số và bao bì
      danSong.mangTanSo[soTanSo] = mangTanSo[soTanSo];
      danSong.mangBaoBi[soTanSo].amLuong = mangAmLuong[soTanSo];
      danSong.mangBaoBi[soTanSo].loai = kBAO_BI__MOC1;
      danSong.mangBaoBi[soTanSo].batDau = 0.0f;
      danSong.mangBaoBi[soTanSo].batDau = 0.01f;
      danSong.mangBaoBi[soTanSo].ketThuc = 0.0f;
      soTanSo++;
   }
   
   return danSong;
}

DanDoiAmLuong datDan_chuong0() {
   
   DanDoiAmLuong danDoiAmLuong;
   
   // ---- đặt các tần số và bao bì
   danDoiAmLuong.tanSo = 1.0f;
   danDoiAmLuong.tanSoChinh = 0.8f;
   danDoiAmLuong.amLuongChinh = 1.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiAmLuong.baoBi.loai = kBAO_BI__MOC0;
   danDoiAmLuong.baoBi.amLuong = 1.0f;
   danDoiAmLuong.baoBi.batDau = 0.0f;
   danDoiAmLuong.baoBi.giua = 0.08f;
   danDoiAmLuong.baoBi.ketThuc = 0.0f;
   
   //   danDoiAmLuong.thoiKetThuc = 0.05f;
   
   return danDoiAmLuong;
}

DanDoiAmLuong datDan_doiAmLuong1() {
   
   DanDoiAmLuong danDoiAmLuong;
   
   // ---- đặt các tần số và bao bì
   danDoiAmLuong.tanSo = 1.0f;
   danDoiAmLuong.tanSoChinh = 2.0f;
   danDoiAmLuong.amLuongChinh = 1.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiAmLuong.baoBi.loai = kBAO_BI__DEU;
   danDoiAmLuong.baoBi.amLuong = 1.0f;
   danDoiAmLuong.baoBi.batDau = 0.0f;
   danDoiAmLuong.baoBi.giua = 1.0f;
   danDoiAmLuong.baoBi.ketThuc = 0.0f;
   
   //   danDoiAmLuong.thoiKetThuc = 0.05f;
   
   return danDoiAmLuong;
}

DanDoiTanSo datDan_chuong1() {
   
   DanDoiTanSo danDoiTanSo;
   danDoiTanSo.soLuongBaoBi = 1;
   
   // ---- đặt các tần số và bao bì
   danDoiTanSo.tanSo[0] = 1.0f;
   danDoiTanSo.tanSoChinh[0] = 15.0f;
   danDoiTanSo.amLuongChinhDau[0] = 0.0f;
   danDoiTanSo.amLuongChinhCuoi[0] = 3.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBi[0].loai = kBAO_BI__MOC0;
   danDoiTanSo.baoBi[0].amLuong = 1.0f;
   danDoiTanSo.baoBi[0].batDau = 0.0f;
   danDoiTanSo.baoBi[0].giua = 0.05f;
   danDoiTanSo.baoBi[0].ketThuc = 0.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBiChinh[0].loai = kBAO_BI__MOC0;
   danDoiTanSo.baoBiChinh[0].amLuong = 1.0f;
   danDoiTanSo.baoBiChinh[0].batDau = 0.0f;
   danDoiTanSo.baoBiChinh[0].giua = 0.05f;
   danDoiTanSo.baoBiChinh[0].ketThuc = 0.0f;
   //   danDoiTanSo.thoiKetThuc = 0.05f;
   
   return danDoiTanSo;
}

DanDoiTanSo datDan_ghen0() {
   
   DanDoiTanSo danDoiTanSo;
   danDoiTanSo.soLuongBaoBi = 1;
   
   // ---- đặt các tần số và bao bì
   danDoiTanSo.tanSo[0] = 1.0f;
   danDoiTanSo.tanSoChinh[0] = 0.333f;
   danDoiTanSo.amLuongChinhDau[0] = 0.0f;
   danDoiTanSo.amLuongChinhCuoi[0] = 2.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBi[0].loai = kBAO_BI__MOC1;
   danDoiTanSo.baoBi[0].amLuong = 1.0f;
   danDoiTanSo.baoBi[0].batDau = 0.0f;
   danDoiTanSo.baoBi[0].giua = 0.2f;
   danDoiTanSo.baoBi[0].ketThuc = 0.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBiChinh[0].loai = kBAO_BI__DEU;
   danDoiTanSo.baoBiChinh[0].amLuong = 1.0f;
   danDoiTanSo.baoBiChinh[0].batDau = 0.0f;
   danDoiTanSo.baoBiChinh[0].giua = 0.2f;
   danDoiTanSo.baoBiChinh[0].ketThuc = 0.0f;
   //   danDoiTanSo.thoiKetThuc = 0.05f;
   
   return danDoiTanSo;
}

DanDoiTanSo datDan_ghen1() {
   
   DanDoiTanSo danDoiTanSo;
   danDoiTanSo.soLuongBaoBi = 1;
   danDoiTanSo.baoBiTuongDoi = 0;
   
   // ---- đặt các tần số và bao bì
   danDoiTanSo.tanSo[0] = 1.0f;
   danDoiTanSo.tanSoChinh[0] = 0.5f;
   danDoiTanSo.amLuongChinhDau[0] = 0.0f;
   danDoiTanSo.amLuongChinhCuoi[0] = 8.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBi[0].loai = kBAO_BI__DOI;
   danDoiTanSo.baoBi[0].amLuong = 1.0f;
   danDoiTanSo.baoBi[0].batDau = 0.0f;
   danDoiTanSo.baoBi[0].giua = 0.2f;
   danDoiTanSo.baoBi[0].ketThuc = 0.5f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBiChinh[0].loai = kBAO_BI__DOI;
   danDoiTanSo.baoBiChinh[0].amLuong = 1.0f;
   danDoiTanSo.baoBiChinh[0].batDau = 0.0f;
   danDoiTanSo.baoBiChinh[0].giua = 0.2f;
   danDoiTanSo.baoBiChinh[0].ketThuc = 0.5f;
   
   return danDoiTanSo;
}

DanDoiTanSo datDan_ghen2() {
   
   DanDoiTanSo danDoiTanSo;
   danDoiTanSo.soLuongBaoBi = 1;
   danDoiTanSo.baoBiTuongDoi = 0;
   
   // ---- đặt các tần số và bao bì
   danDoiTanSo.tanSo[0] = 1.0f;
   danDoiTanSo.tanSoChinh[0] = 1.99f;//(float)(rand() % 30)/5.0f;
   danDoiTanSo.amLuongChinhDau[0] = 1.0f;//(rand() % 100)/10.0f;
   danDoiTanSo.amLuongChinhCuoi[0] = 1.5f;//(rand() % 100)/10.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBi[0].loai = kBAO_BI__DOI;
   danDoiTanSo.baoBi[0].amLuong = 1.0f;
   danDoiTanSo.baoBi[0].batDau = 0.0f;
   danDoiTanSo.baoBi[0].giua = 0.1f;
   danDoiTanSo.baoBi[0].ketThuc = 0.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBiChinh[0].loai = kBAO_BI__DOI;
   danDoiTanSo.baoBiChinh[0].amLuong = 1.0f;
   danDoiTanSo.baoBiChinh[0].batDau = 0.0f;
   danDoiTanSo.baoBiChinh[0].giua = 0.1f;
   danDoiTanSo.baoBiChinh[0].ketThuc = 0.0f;
   
   return danDoiTanSo;
}


DanDoiTanSo datDan_doiTanSo4() {
   
   DanDoiTanSo danDoiTanSo;
   danDoiTanSo.soLuongBaoBi = 1;
   danDoiTanSo.baoBiTuongDoi = 0;
   
   // ---- đặt các tần số và bao bì
   danDoiTanSo.tanSo[0] = 1.0f;
   danDoiTanSo.tanSoChinh[0] = 1.4f;
   danDoiTanSo.amLuongChinhDau[0] = 0.0f;
   danDoiTanSo.amLuongChinhCuoi[0] = 10.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBi[0].loai = kBAO_BI__DOI;
   danDoiTanSo.baoBi[0].amLuong = 1.0f;
   danDoiTanSo.baoBi[0].batDau = 0.0f;
   danDoiTanSo.baoBi[0].giua = 0.1f;
   danDoiTanSo.baoBi[0].ketThuc = 0.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSo.baoBiChinh[0].loai = kBAO_BI__DOI;
   danDoiTanSo.baoBiChinh[0].amLuong = 1.0f;
   danDoiTanSo.baoBiChinh[0].batDau = 0.0f;
   danDoiTanSo.baoBiChinh[0].giua = 0.1f;
   danDoiTanSo.baoBiChinh[0].ketThuc = 0.0f;
   
   return danDoiTanSo;
}


DanDoiTanSo datDan_doiTanSo5() {
   
   DanDoiTanSo danDoiTanSo;
   danDoiTanSo.soLuongBaoBi = 3;
   danDoiTanSo.baoBiTuongDoi = 1;
   
   float amLuong[5] = {1.0f, 1.0f, 1.0f, 0.0f, 0.0f};
   float tanSo[5] = {1.0f, 1.525f, 2.0f, 0.0f, 0.0f};
   float tanSoChinh[5] = {2.4f, 3.66f, 4.2f, 0.0f, 0.0f};
   
   unsigned char soTanSo = 0;
   while( soTanSo < 3 ) {
      // ---- đặt các tần số và bao bì
      danDoiTanSo.tanSo[soTanSo] = tanSo[soTanSo];
      danDoiTanSo.tanSoChinh[soTanSo] = tanSoChinh[soTanSo];
      danDoiTanSo.amLuongChinhDau[soTanSo] = 0;
      danDoiTanSo.amLuongChinhCuoi[soTanSo] = 25.0f;
      
      // ---- thời gian giảm/kết thúc nốt
      danDoiTanSo.baoBi[soTanSo].loai = kBAO_BI__MOC1;
      danDoiTanSo.baoBi[soTanSo].amLuong = 1.0f;
      danDoiTanSo.baoBi[soTanSo].batDau = 0.0f;
      danDoiTanSo.baoBi[soTanSo].giua = 0.1f;
      danDoiTanSo.baoBi[soTanSo].ketThuc = 0.0f;

      // ---- thời gian giảm/kết thúc nốt
      danDoiTanSo.baoBiChinh[soTanSo].loai = kBAO_BI__MOC0;
      danDoiTanSo.baoBiChinh[soTanSo].amLuong = 1.0f;
      danDoiTanSo.baoBiChinh[soTanSo].batDau = 0.1f;
      danDoiTanSo.baoBiChinh[soTanSo].ketThuc = 0.0f;
      soTanSo++;
   }
   
   return danDoiTanSo;
}


DanDoiTanSoCap datDan_doiTanSoCapNgauNhien() {
   
   DanDoiTanSoCap danDoiTanSoCap;
   
   danDoiTanSoCap.soLuongBaoBi = 2;
   
   unsigned short soBaoBi = 0;
   while( soBaoBi < 3 ) {
   
   // ---- đặt các tần số và bao bì
      if( soBaoBi == 0 )
         danDoiTanSoCap.tanSoChinh0[soBaoBi] = 1.0f;
      else
         danDoiTanSoCap.tanSoChinh0[soBaoBi] = (rand() % 1000)*0.01f;
      
   danDoiTanSoCap.tanSoChinh1[soBaoBi] = (rand() % 1000)*0.01f;
   danDoiTanSoCap.amLuongChinh0[soBaoBi] = (rand() % 1000)*0.004f;
   danDoiTanSoCap.amLuongChinh1[soBaoBi] = (rand() % 1000)*0.004f;
   printf( "%d tanSoChinh0 %5.3f  AL chinh0 %5.3f  tanSoChinh1 %5.3f  AL chinh1 %5.3f\n", soBaoBi,
          danDoiTanSoCap.tanSoChinh0[soBaoBi], danDoiTanSoCap.amLuongChinh0[soBaoBi],
          danDoiTanSoCap.tanSoChinh1[soBaoBi], danDoiTanSoCap.amLuongChinh1[soBaoBi] );
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSoCap.baoBi[soBaoBi].loai = rand() % 6;
   danDoiTanSoCap.baoBi[soBaoBi].amLuong = (rand() % 1000)*0.001f;
   danDoiTanSoCap.baoBi[soBaoBi].batDau = 0.0f;
   danDoiTanSoCap.baoBi[soBaoBi].ha = 0.05f;
   danDoiTanSoCap.baoBi[soBaoBi].giua = 0.1f;
   danDoiTanSoCap.baoBi[soBaoBi].ketThuc = 0.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSoCap.baoBiChinh0[soBaoBi].loai = rand() % 6;
   danDoiTanSoCap.baoBiChinh0[soBaoBi].amLuong = (rand() % 1000)*0.001f;
   danDoiTanSoCap.baoBiChinh0[soBaoBi].batDau = 0.0f;
   danDoiTanSoCap.baoBiChinh0[soBaoBi].ha = 0.02f;
   danDoiTanSoCap.baoBiChinh0[soBaoBi].giua = 0.05f;
   danDoiTanSoCap.baoBiChinh0[soBaoBi].ketThuc = 0.0f;
   
   // ---- thời gian giảm/kết thúc nốt
   danDoiTanSoCap.baoBiChinh1[soBaoBi].loai = rand() % 6;
   danDoiTanSoCap.baoBiChinh1[soBaoBi].amLuong = (rand() % 1000)*0.001f;
   danDoiTanSoCap.baoBiChinh1[soBaoBi].batDau = 0.0f;
   danDoiTanSoCap.baoBiChinh1[soBaoBi].ha = 0.0f;
   danDoiTanSoCap.baoBiChinh1[soBaoBi].giua = 0.05f;
   danDoiTanSoCap.baoBiChinh1[soBaoBi].ketThuc = 0.0f;

   printf( " %d danbaoBi.loai %d  --  chinh0.loai %d  AL %5.3f -- chinh1.loai %d  AL %5.3f\n", soBaoBi,
          danDoiTanSoCap.baoBi[0].loai, danDoiTanSoCap.baoBiChinh0[soBaoBi].loai, danDoiTanSoCap.baoBiChinh0[soBaoBi].amLuong, danDoiTanSoCap.baoBiChinh1[soBaoBi].loai, danDoiTanSoCap.baoBiChinh1[soBaoBi].amLuong );

      soBaoBi++;
   }
   //   danDoiTanSo.thoiKetThuc = 1.0f;  // <---- không xài?
   
   return danDoiTanSoCap;
}

DanHuyenNaoTrang datDanHuyenNao0() {

   DanHuyenNaoTrang danHuyenNaoTrang;
   
   // ---- đặt các tần số và bao bì
   danHuyenNaoTrang.amLuong = 1.0f;

   // ---- thời gian giảm/kết thúc nốt
   danHuyenNaoTrang.baoBi.loai = kBAO_BI__DOI;
   danHuyenNaoTrang.baoBi.amLuong = 1.0f;
   danHuyenNaoTrang.baoBi.batDau = 0.0f;
   danHuyenNaoTrang.baoBi.giua = 0.8f;
   danHuyenNaoTrang.baoBi.ketThuc = 0.0f;
   
   return danHuyenNaoTrang;
}


#pragma mark ---- Đọc Tham Số
void docThamSoPhimTruong( int argc, char **argv, unsigned int *soPhimTruong ) {
   
   if( argc > 1 ) {
      sscanf( argv[1], "%u", soPhimTruong );
      // ---- kiểm tra sốPhimTrường
      if( *soPhimTruong >= 3 )
         *soPhimTruong = 2;
   }
   else {
      *soPhimTruong = 0;
   }
}


#pragma mark ---- ÂM NHẠC TRƯỜNG 0
unsigned short datTrongPhimTruong0( Not *mangNot, float *tongThoiGian );
// ---- Trống
unsigned short datNotNhip_trong( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong, float cachNhip, unsigned short soLapLai );
// ---- Ghẹn To
unsigned short datGhenToPhimTruong0( Not *mangNot, float *tongThoiGian );
unsigned short datNotChoGhenTo( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong, unsigned short doan, unsigned short soNot, float hatBaoLau );
// ---- Chuông
unsigned short datChuongPhimTruong0( Not *mangNot, float *tongThoiGian );
unsigned short datNotChuongLapLai0( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong, float cachNhip, unsigned short soLapLai, unsigned short tamChuong );
unsigned short datNotNhip_nhanh0( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong );
// ---- Chẹn Điện Tử
unsigned short datAmThanhTrungLong( Not *mangNot, float *tongThoiGian );
unsigned short datNotChoLanGhenDienTu( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong );
// ---- Đàn Dây 0
unsigned short datNotChoDanDay0_phimTruong0( Not *mangNot, float *thoiGian );
// ---- Đàn Dây 1
unsigned short datNotChoDanDay1_phimTruong0( Not *mangNot, float *thoiGian );

// ---- Sáo
unsigned short datNotChoSao( Not *mangNot, float *tongThoiGian );

// ---- chép lại
unsigned short chepLaiNot( Not *mangNot, unsigned char *mangSoDoan, unsigned char *mangSoNot, float *mangCachNot, float *mangHatNotBaoLau, unsigned short soLuongNot, unsigned short soLuongLapLai, float thoiBatDauHat, float *thoiGian, float amLuong );

unsigned short chepLaiNot_chinhNgauNhien( Not *mangNot, unsigned char *mangSoDoan, unsigned char *mangSoNot, float *mangCachNot, float *mangHatNotBaoLau, unsigned short soLuongNot, unsigned short soLuongLapLai, float thoiBatDauHat, float *thoiGian, float amLuong );

// ---- ngẫu nhiên
unsigned short datNotChoSaoLapLaiNgauNhien( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong,
                                           float doanThoiGian, unsigned short soLanLapLai );


// tanSoChinh 0.200   AL chinh Dau 8.000  AL chinh Cuoi 8.000
// tanSoChinh 1.400   AL chinh Dau 10.000  AL chinh Cuoi 0.000
// tanSoChinh 1.000   AL chinh Dau 10.000  AL chinh Cuoi 3.600
// tanSoChinh 0.400   AL chinh Dau 0.400  AL chinh Cuoi 3.200
// tanSoChinh 1.600   AL chinh Dau 10.000  AL chinh Cuoi 3.200
AmNhacTruong datAmNhacTruong0() {
   
   float thoiGianHat = 66.0f;

   AmNhacTruong amNhacTruong;
   // ---- dữ liệu âm nhạc
   amNhacTruong.tongThoiGianHat = thoiGianHat;
   amNhacTruong.duLieuAmThanh.soLuongKenh = 2;
   amNhacTruong.duLieuAmThanh.tocDoMauVat = hTOC_DO_MAU_VAT_MAC_DINH_48000;
   unsigned int soLuongMauVat = hTOC_DO_MAU_VAT_MAC_DINH_48000*thoiGianHat;
   amNhacTruong.duLieuAmThanh.soLuongMauVat = soLuongMauVat;  // chứng nào có nốt nhạc mới có thê tính này
   amNhacTruong.duLieuAmThanh.soLuongKenh = 2;  // nhị kênh
   amNhacTruong.duLieuAmThanh.mangMauVatKenh0 = malloc( sizeof( float )*soLuongMauVat );
   amNhacTruong.duLieuAmThanh.mangMauVatKenh1 = malloc( sizeof( float )*soLuongMauVat );

   amNhacTruong.soLuongLan = 6;
   amNhacTruong.danhSachLan = malloc( sizeof( Lan )*amNhacTruong.soLuongLan );

   // ---- làn 0 - trống
   unsigned short soLuongNot = 0;
   float thoiGian = 0.0f;
   amNhacTruong.danhSachLan[0].dan.loai = kKIEU_DAN__DOI_TAN_SO;
   amNhacTruong.danhSachLan[0].dan.kieuDan.danDoiTanSo = datDan_doiTanSo5(); // ÂM LƯỚNG RẤT LỚN
   amNhacTruong.danhSachLan[0].soLuongNot = datTrongPhimTruong0( amNhacTruong.danhSachLan[0].mangNot, &thoiGian );

   // ---- làn 1 - chuông
   soLuongNot = 0;
   thoiGian = 38.7f;
   amNhacTruong.danhSachLan[1].dan.loai = kKIEU_DAN__DOI_AM_LUONG;
   amNhacTruong.danhSachLan[1].dan.kieuDan.danDoiAmLuong = datDan_chuong0();
   amNhacTruong.danhSachLan[1].soLuongNot = datChuongPhimTruong0( amNhacTruong.danhSachLan[1].mangNot, &thoiGian );

   // ---- làn 2
   soLuongNot = 0;
   thoiGian = 0.0f;
   amNhacTruong.danhSachLan[2].dan.loai = kKIEU_DAN__DOI_TAN_SO;
   amNhacTruong.danhSachLan[2].dan.kieuDan.danDoiTanSo = datDan_ghen1();
   amNhacTruong.danhSachLan[2].soLuongNot = datGhenToPhimTruong0( amNhacTruong.danhSachLan[2].mangNot, &thoiGian );
//   printf( "soLuongNot ghen1 %d  thoiGian %5.3f\n", amNhacTruong.danhSachLan[2].soLuongNot, thoiGian );

   // ---- làn 3 - âm thanh trúng lồng sao gai
   soLuongNot = 0;
   thoiGian = 26.8f;
   amNhacTruong.danhSachLan[3].dan.loai = kKIEU_DAN__DOI_TAN_SO;
   amNhacTruong.danhSachLan[3].dan.kieuDan.danDoiTanSo = datDan_doiTanSo4();
   amNhacTruong.danhSachLan[3].soLuongNot = datAmThanhTrungLong( amNhacTruong.danhSachLan[3].mangNot, &thoiGian );

   // ---- làn 4
   soLuongNot = 0;
   thoiGian = 0.0f;
   amNhacTruong.danhSachLan[4].dan.loai = kKIEU_DAN__SONG;
   amNhacTruong.danhSachLan[4].dan.kieuDan.danSong = datDan_danDay0();
   amNhacTruong.danhSachLan[4].soLuongNot = datNotChoDanDay0_phimTruong0( &(amNhacTruong.danhSachLan[4].mangNot[soLuongNot]), &thoiGian );

   // ---- làn 5
   soLuongNot = 0;
   thoiGian = 4.0f;
   amNhacTruong.danhSachLan[5].dan.loai = kKIEU_DAN__SONG;
   amNhacTruong.danhSachLan[5].dan.kieuDan.danSong = datDan_danDay1();
   amNhacTruong.danhSachLan[5].soLuongNot = datNotChoDanDay1_phimTruong0( &(amNhacTruong.danhSachLan[5].mangNot[soLuongNot]), &thoiGian );

   return amNhacTruong;
}


#pragma mark ---- Trống
unsigned short datTrongPhimTruong0( Not *mangNot, float *tongThoiGian ){
   
   unsigned short soLuongNot = 0;
   float thoiGian = 0.0f;
   float amLuongTongQuat = 0.2f; // âm tanh rất lớn, giúp quản lý nó
   
   soLuongNot += datNotNhip_trong( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.03f*amLuongTongQuat, 1.0f, 10 );
   printf( "trong - soLuongNot %d  thoiGian %5.3f\n", soLuongNot, thoiGian );
   
   // ---- vòng xuyên
   soLuongNot += datNotNhip_trong( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.05f*amLuongTongQuat, 0.5f, 32 );
   printf( "trong - soLuongNot %d  thoiGian %5.3f\n", soLuongNot, thoiGian );
   
   // ---- trái banh chánh bay lên trời
   soLuongNot += datNotNhip_trong( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.05f*amLuongTongQuat, 1.5f, 6 );
   printf( "trong - soLuongNot %d  thoiGian %5.3f\n", soLuongNot, thoiGian );
   
   // ---- chạy trống
   soLuongNot += datNotNhip_trong( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.4f*amLuongTongQuat, 0.4f, 60 );

   // ---- giảm âm lượng khi gần hết
   unsigned short soNot = 0;
   float amLuong = 0.4f*amLuongTongQuat;
   while( soNot < 30 ) {
      soLuongNot += datNotNhip_trong( &(mangNot[soLuongNot]), thoiGian, &thoiGian, amLuong, 0.4f, 1 );
      amLuong -= 0.01f*amLuongTongQuat;
      soNot++;
   }

   *tongThoiGian = thoiGian;
   return soLuongNot;
}

/* nhịp trống */
unsigned short datNotNhip_trong( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong, float cachNhip, unsigned short soLapLai ) {
   
   float thoiGianHat = thoiBatDauHat;

   unsigned short soDayNot = 0;  // số nốt trong dãy nốt
   while ( soDayNot < soLapLai ) {

      mangNot[soDayNot].giaiDieu = kGIAI_DIEU_5;
      mangNot[soDayNot].doan = 0;
      mangNot[soDayNot].so = 0;
      
      mangNot[soDayNot].thoiHat = thoiGianHat;
      mangNot[soDayNot].hatBaoLau = 0.2f;
      
      mangNot[soDayNot].amLuong = amLuong;
      if( soDayNot & 1 ) {
         mangNot[soDayNot].viTriTraiTrai = 1.0f;
         mangNot[soDayNot].viTriTraiPhai = 0.5f;
      }
      else {
         mangNot[soDayNot].viTriTraiTrai = 0.5f;
         mangNot[soDayNot].viTriTraiPhai = 1.0f;
      }
      thoiGianHat += cachNhip;
      soDayNot++;
   }

   *thoiGian = thoiGianHat;
   return soDayNot;
}

#pragma mark ---- Ghẹn To
unsigned short datGhenToPhimTruong0( Not *mangNot, float *tongThoiGian ) {

   unsigned soLuongNot = 0;
   float thoiGian = 0.0f;
   // ---- ăn trái banh
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 39.5f, &thoiGian, 0.15f, 1, 0, 3.0f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 39.5f, &thoiGian, 0.10f, 0, 0, 1.0f );
   // ---- vào vòng xuyên
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 42.5f, &thoiGian, 0.25f, 0, 0, 10.0f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 42.5f, &thoiGian, 0.15f, -1, 0, 8.0f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 48.0f, &thoiGian, 0.3f, 0, 3, 1.3f );
   
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 49.5f, &thoiGian, 0.25f, 0, 3, 1.3f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 49.5f, &thoiGian, 0.15f, -1, 3, 1.0f );
   
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 51.0f, &thoiGian, 0.2f, 1, 2, 2.5f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 56.0f, &thoiGian, 0.2f, 1, 2, 2.5f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 56.5f, &thoiGian, 0.16f, 0, 2, 1.3f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 57.0f, &thoiGian, 0.3f, 0, 2, 0.5f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 57.5f, &thoiGian, 0.3f, 0, 2, 0.5f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 58.0f, &thoiGian, 0.3f, 0, 2, 0.5f );

   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), 59.5f, &thoiGian, 0.3f, 0, 1, 1.5f );

   return soLuongNot;
}

unsigned short datNotChoGhenTo( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong, unsigned short doan, unsigned short soNot, float hatBaoLau ) {
   
   float thoiGianHat = thoiBatDauHat;
      
   mangNot[0].giaiDieu = kGIAI_DIEU_5;
   mangNot[0].doan = doan;
   mangNot[0].so = soNot;
   
   mangNot[0].thoiHat = thoiGianHat;
   mangNot[0].hatBaoLau = hatBaoLau;
   
   mangNot[0].amLuong = amLuong;
   
   mangNot[0].viTriTraiTrai = 1.0f;
   mangNot[0].viTriTraiPhai = 1.0f;

   thoiGianHat += hatBaoLau;

   *thoiGian = thoiGianHat;
   return 1;
}

unsigned short datNotNhip_nhanh0( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong ) {
   
   float thoiGianHat = thoiBatDauHat;
   
   char danhSachSoNot[] = {0, 1, 2, 3, 4, 0, 1, 0, 4, 3, 2, 1, 0, 1, 2, 3, 4, 0, 1, 0, 4, 3, 2, 1};
   //   char danhSachSoDoan[] = {-1, -1, -1, -1, -1, 0, 0, 0, -1, -1, -1, -1};
   
   unsigned short soDayNot = 0;  // số nốt trong dãy nốt
   while ( soDayNot < 24 ) {
      
      mangNot[soDayNot].giaiDieu = kGIAI_DIEU_5;
      mangNot[soDayNot].doan = -1;
      mangNot[soDayNot].so = danhSachSoNot[soDayNot];
      
      mangNot[soDayNot].thoiHat = thoiGianHat;
      mangNot[soDayNot].hatBaoLau = 0.25f;
      
      mangNot[soDayNot].amLuong = amLuong;
      if( soDayNot & 1 ) {
         mangNot[soDayNot].viTriTraiTrai = 1.0f;
         mangNot[soDayNot].viTriTraiPhai = 0.2f;
      }
      else {
         mangNot[soDayNot].viTriTraiTrai = 0.2f;
         mangNot[soDayNot].viTriTraiPhai = 1.0f;
      }
      thoiGianHat += 0.1f;
      soDayNot++;
   }
   
   *thoiGian = thoiGianHat;
   return soDayNot;
}

#pragma mark ---- Chuông
unsigned short datChuongPhimTruong0( Not *mangNot, float *tongThoiGian ) {
   
   unsigned short soLuongNot = 0;
   float thoiGian = 38.1f;

   soLuongNot += datNotChuongLapLai0( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.1f, 0.4f, 16, 16 );
   soLuongNot += datNotChuongLapLai0( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.1f, 0.4f, 8, 8 );
   soLuongNot += datNotChuongLapLai0( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.1f, 0.4f, 36, 4 );

   printf( "chuong - thoiGian %5.3f\n", thoiGian );
   // ---- giảm âm lượng khi gần hết
   unsigned short soNot = 0;
   float amLuong = 0.1f;
   while( soNot < 30 ) {
      soLuongNot += datNotChuongLapLai0( &(mangNot[soLuongNot]), thoiGian, &thoiGian, amLuong, 0.4f, 1, 4 );
      amLuong -= 0.005f;
      if( amLuong < 0.0f )
         amLuong = 0.0f;
      soNot++;
   }
//   printf( "chuong - thoiGian %5.3f\n", thoiGian );
   *tongThoiGian = thoiGian;
   
   return soLuongNot;
}


// nên đặt tamChuong ≥ soLapLai
unsigned short datNotChuongLapLai0( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong, float cachNhip, unsigned short soLapLai, unsigned short tamChuong ) {
   
   float thoiGianHat = thoiBatDauHat;
   
   unsigned short soDayNot = 0;  // số nốt trong dãy nốt
   while ( soDayNot < soLapLai ) {
      
      mangNot[soDayNot].giaiDieu = kGIAI_DIEU_5;
      mangNot[soDayNot].doan = 6;
      mangNot[soDayNot].so = 0;

      mangNot[soDayNot].thoiHat = thoiGianHat;
      mangNot[soDayNot].hatBaoLau = 0.2f;
      
      mangNot[soDayNot].amLuong = amLuong;
      if( soDayNot & 1 ) {
         mangNot[soDayNot].viTriTraiTrai = 1.0f;
         mangNot[soDayNot].viTriTraiPhai = 0.5f;
      }
      else {
         mangNot[soDayNot].viTriTraiTrai = 0.5f;
         mangNot[soDayNot].viTriTraiPhai = 1.0f;
      }
      thoiGianHat += cachNhip;
      soDayNot++;
      
      if( (soDayNot % tamChuong) == (tamChuong-1) ) {
         mangNot[soDayNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soDayNot].doan = 6;
         mangNot[soDayNot].so = 2;
         
         mangNot[soDayNot].thoiHat = thoiGianHat;
         mangNot[soDayNot].hatBaoLau = 0.2f;
         
         mangNot[soDayNot].amLuong = amLuong;
  
         mangNot[soDayNot].viTriTraiTrai = 0.7f;
         mangNot[soDayNot].viTriTraiPhai = 0.7f;
         thoiGianHat += cachNhip*0.25f;
         soDayNot++;
         // ----
         mangNot[soDayNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soDayNot].doan = 6;
         mangNot[soDayNot].so = 4;
         
         mangNot[soDayNot].thoiHat = thoiGianHat;
         mangNot[soDayNot].hatBaoLau = 0.2f;
         
         mangNot[soDayNot].amLuong = amLuong;
         
         mangNot[soDayNot].viTriTraiTrai = 0.7f;
         mangNot[soDayNot].viTriTraiPhai = 0.7f;
         thoiGianHat += cachNhip*0.25f;
         soDayNot++;
         // ----
         mangNot[soDayNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soDayNot].doan = 6;
         mangNot[soDayNot].so = 3;
         
         mangNot[soDayNot].thoiHat = thoiGianHat;
         mangNot[soDayNot].hatBaoLau = 0.2f;
         
         mangNot[soDayNot].amLuong = amLuong;
         
         mangNot[soDayNot].viTriTraiTrai = 0.7f;
         mangNot[soDayNot].viTriTraiPhai = 0.7f;
         thoiGianHat += cachNhip*0.50f;
         soDayNot++;
      }
   }
   
   *thoiGian = thoiGianHat;
   return soDayNot;
}

#pragma mark ---- Âm Thanh Trúng Lồng
unsigned short datAmThanhTrungLong( Not *mangNot, float *tongThoiGian ) {
   
   unsigned short soLuongNot = 0;
   float thoiGian = 26.8f;

   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.3f, 2, 0, .05f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.3f, 2, 2, .05f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.3f, 2, 4, .05f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.3f, 3, 1, .05f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.3f, 3, 3, .05f );
   soLuongNot += datNotChoGhenTo( &(mangNot[soLuongNot]), thoiGian, &thoiGian, 0.3f, 4, 0, .05f );

   return soLuongNot;
}


#pragma mark ---- Phim Trường 0 - Đàn Dây 0
unsigned short datNotChoDanDay0_phimTruong0( Not *mangNot, float *thoiGian ) {
   
   float tongThoiGian = 0.0f;
   unsigned short soLuongNot = 0;

   // ---- ngoài buền bay đến vòng xuyên
   unsigned char soDoanBien[16] = {2, 2, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 2, 1, 2};
   unsigned char soNotBien[16] = {4, 4, 2, 3, 2, 2, 2, 3, 1, 3, 0, 1, 2, 2, 2, 0};
   float cachNotBien[16] = {0.250f, 0.250f, 0.250f, 0.250f, 0.125f, 0.125f, 0.125f, 0.375f,
      0.250f, 0.250f, 0.250f, 0.375f, 0.375f, 0.250f, 0.250f, 0.250f};
   float hatNotBaoLauBien[16] = {0.250f, 0.250f, 0.250f, 0.250f, 0.125f, 0.125f, 0.125f, 0.125f,
      0.250f, 0.250f, 0.250f, 0.250f, 0.125f, 0.125f, 0.125f, 0.125f};
   
   soLuongNot += chepLaiNot_chinhNgauNhien(&(mangNot[soLuongNot]), soDoanBien, soNotBien,
                                     cachNotBien, hatNotBaoLauBien, 16, 3, tongThoiGian, &tongThoiGian, 0.4f );
//   printf( "sao  tongThoiGian %5.3f soLuongNot %d\n", tongThoiGian, soLuongNot );
   
   unsigned char soDoanVongXuyen[16] = {3, 2, 4, 1, 1, 2, 1, 3, 3, 4, 2, 4, 4, 2, 1, 2};
   unsigned char soNotVongXuyen[16] = {3, 4, 2, 2, 4, 1, 3, 4, 4, 1, 0, 2, 0, 2, 2, 0};
   float cachNotVongXuyen[16] = {0.250f, 0.250f, 0.500f, 0.125f, 0.125f, 0.250f, 0.250f, 0.250f,
      0.250f, 0.250f, 0.125f, 0.125f, 0.5f, 0.250f, 0.250f, 0.250f};
   float hatNotBaoLauVongXuyen[16] = {0.250f, 0.250f, 0.250f, 0.125f, 0.125f, 0.250f, 0.250f, 0.250f,
      0.125f, 0.250f, 0.125f, 0.250f, 0.125f, 0.125f, 0.125f, 0.125f};
   soLuongNot += chepLaiNot_chinhNgauNhien(&(mangNot[soLuongNot]), soDoanVongXuyen, soNotVongXuyen,
                                     cachNotVongXuyen, hatNotBaoLauVongXuyen, 13, 5, tongThoiGian, &tongThoiGian, 0.4f );

   unsigned char soDoanBayLenTroi[21] = {2, 1, 2, 2, 1, 3, 1, 3, 2, 1, 3, 1, 2, 3, 1, 2, 3, 1, 2, 2, 1};
   unsigned char soNotBayLenTroi[21] = {4, 1, 4, 1, 2, 4, 2, 4, 0, 2, 3, 2, 1, 1, 2, 0, 2, 3, 4, 0, 2};
   float cachNotBayLenTroi[21] = {0.500f, 0.500f, 0.750f, 0.500f, 0.250f, 0.250f, 0.250f, 0.250f,
      0.250f, 0.500f, 0.250f, 0.500f, 0.500f, 0.500f, 0.250f, 0.250f,
      0.500f, 0.250f, 0.500f, 0.250f, 0.250f};
   float hatNotBaoLauBayLenTroi[21] = {0.50f, 0.50f, 0.50f, 0.50f, 0.25f, 0.25f, 0.25f, 0.25f,
      0.25f, 0.50f, 0.25f, 0.50f, 0.50f, 0.50f, 0.25f, 0.50f, 0.25f, 0.25f, 0.50f, 0.25f, 0.25f};
   soLuongNot += chepLaiNot_chinhNgauNhien(&(mangNot[soLuongNot]), soDoanBayLenTroi, soNotBayLenTroi,
                                     cachNotBayLenTroi, hatNotBaoLauBayLenTroi, 21, 1, tongThoiGian, &tongThoiGian, 0.3f );
   
   // ---- hai nốt cuối
   mangNot[soLuongNot].giaiDieu = kGIAI_DIEU_5;
   mangNot[soLuongNot].doan = 2;
   mangNot[soLuongNot].so = 3;
   
   mangNot[soLuongNot].thoiHat = tongThoiGian;
   mangNot[soLuongNot].hatBaoLau = 0.25f;
   
   mangNot[soLuongNot].amLuong = 0.3f*0.5f;
   mangNot[soLuongNot].viTriTraiTrai = 0.5f;
   mangNot[soLuongNot].viTriTraiPhai = 1.0f;
   tongThoiGian += 0.25f;
   soLuongNot++;
   
   mangNot[soLuongNot].giaiDieu = kGIAI_DIEU_5;
   mangNot[soLuongNot].doan = 2;
   mangNot[soLuongNot].so = 3;
   
   mangNot[soLuongNot].thoiHat = tongThoiGian;
   mangNot[soLuongNot].hatBaoLau = 0.75f;
   
   mangNot[soLuongNot].amLuong = 0.3f*0.5f;
   mangNot[soLuongNot].viTriTraiTrai = 0.5f;
   mangNot[soLuongNot].viTriTraiPhai = 1.0f;
   tongThoiGian += 0.25f;
   soLuongNot++;
//   printf( "sao  tongThoiGian %5.3f soLuongNot %d\n", tongThoiGian, soLuongNot );

   *thoiGian = tongThoiGian;
   return soLuongNot;
}

#pragma mark ---- Phim Trường 0 - Dây 1
unsigned short datNotChoDanDay1_phimTruong0( Not *mangNot, float *thoiGian ) {
   
   float tongThoiGian = 0.0f;
   unsigned short soLuongNot = 0;
   
   // ---- ngoài buền bay đến vòng xuyên
   unsigned char soDoanBien[20] = {0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0};
   unsigned char soNotBien[20] = {4, 3, 2, 3, 4, 0,   4, 3, 2, 3, 4, 0,  4, 3, 2, 3, 4, 0,  4, 3};
   float cachNotBien[20] = {6.0f, 6.0f, 6.0f, 6.0f, 6.0f, 6.0f, 4.0, 2.0f,
                             2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f,
                             2.0f, 2.0f, 2.0f, 2.0f};
   float hatNotBaoLauBien[20] = {8.0f, 8.0f, 8.0f, 8.0f, 8.0f, 8.0f, 6.0f, 4.0f,
                             4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f,
                             4.0f, 4.0f, 4.0f, 4.0f};
   
   //   soLuongNot += chepLaiNot(&(mangNot[soLuongNot]), soDoanBien, soNotBien,
   //                            cachNotBien, hatNotBaoLauBien, 17, 3, tongThoiGian, &tongThoiGian, 0.4f );
   
   soLuongNot += chepLaiNot( &(mangNot[soLuongNot]), soDoanBien, soNotBien,
                            cachNotBien, hatNotBaoLauBien, 18, 1, tongThoiGian, &tongThoiGian, 0.7f );
   
//   printf( "day1  tongThoiGian %5.3f soLuongNot %d\n", tongThoiGian, soLuongNot );
   
   *thoiGian = tongThoiGian;
   return soLuongNot;
}


#pragma mark ---- Sáo
/*unsigned short datNotChoSao( Not *mangNot, float *thoiGian ) {
   
   float tongThoiGian = 3.0f;
   unsigned short soLuongNot = 0;
   
   // ---- ngoài buền bay đến vòng xuyên
   unsigned char soDoanBien[17] = {4, 3, 4, 4, 4, 4,   4, 2, 4, 2, 2, 4, 2, 4, 3, 4, 3};
   unsigned char soNotBien[17] = {4, 2, 4, 0, 3, 0,   2, 2, 2, 0, 1, 0, 0, 0, 2, 1, 1};
   float cachNotBien[17] = {0.250f, 0.250f, 0.250f, 0.125f, 0.250f, 0.500f,   0.250f, 0.250f,
      0.250f, 0.125f, 0.125f, 0.125f, 0.250f, 0.125f, 0.250f, 0.625f};
   float hatNotBaoLauBien[17] = {0.125f, 0.250f, 0.250f, 0.125f, 0.250f, 0.250f,   0.250f, 0.250f,
      0.250f, 0.125f, 0.125f, 0.125f, 0.250f, 0.125f, 0.250f, 0.250f, 0.125f};
   
//   soLuongNot += chepLaiNot(&(mangNot[soLuongNot]), soDoanBien, soNotBien,
//                            cachNotBien, hatNotBaoLauBien, 17, 3, tongThoiGian, &tongThoiGian, 0.4f );
   
   soLuongNot += chepLaiNot_chinhNgauNhien( &(mangNot[soLuongNot]), soDoanBien, soNotBien,
                                           cachNotBien, hatNotBaoLauBien, 17, 3, tongThoiGian, &tongThoiGian, 0.4f );

   printf( "sao  tongThoiGian %5.3f soLuongNot %d\n", tongThoiGian, soLuongNot );

   *thoiGian = tongThoiGian;
   return soLuongNot;
} */



#pragma mark ---- CHÉP LẠI
unsigned short chepLaiNot( Not *mangNot, unsigned char *mangSoDoan, unsigned char *mangSoNot, float *mangCachNot, float *mangHatNotBaoLau, unsigned short soLuongNot, unsigned short soLuongLapLai, float thoiBatDauHat, float *thoiGian, float amLuong ) {
   
   unsigned short soNot = 0;
   float thoiGianHat = thoiBatDauHat;
   
   unsigned short soLapLai = 0;
   
   while ( soLapLai < soLuongLapLai ) {
      unsigned short chiSo = 0;
      // ----
      while( chiSo < soLuongNot ) {
         //        printf( "chiSo %d - %d %d  thoiHat %5.3f  hatBaoLau %5.3f\n", chiSo, mangSoDoan[chiSo], mangSoNot[chiSo], thoiGianHat, mangHatNotBaoLau[chiSo] );
         mangNot[soNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soNot].doan = mangSoDoan[chiSo];
         mangNot[soNot].so = mangSoNot[chiSo];
         
         mangNot[soNot].thoiHat = thoiGianHat;
         mangNot[soNot].hatBaoLau = mangHatNotBaoLau[chiSo];
         
         mangNot[soNot].amLuong = amLuong*0.5f;
         mangNot[soNot].viTriTraiTrai = 0.5f;
         mangNot[soNot].viTriTraiPhai = 1.0f;
         soNot++;
         
         mangNot[soNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soNot].doan = mangSoDoan[chiSo] - 1;
         mangNot[soNot].so = mangSoNot[chiSo];
         
         mangNot[soNot].thoiHat = thoiGianHat;
         mangNot[soNot].hatBaoLau = mangHatNotBaoLau[chiSo];
         
         mangNot[soNot].amLuong = amLuong*0.5f;
         mangNot[soNot].viTriTraiTrai = 1.0f;
         mangNot[soNot].viTriTraiPhai = 0.5f;
         soNot++;
         
         thoiGianHat += mangCachNot[chiSo];
         chiSo++;
      }
      soLapLai++;
   }
   *thoiGian = thoiGianHat;
   
   return soNot;
}

unsigned short chepLaiNot_chinhNgauNhien( Not *mangNot, unsigned char *mangSoDoan, unsigned char *mangSoNot, float *mangCachNot, float *mangHatNotBaoLau, unsigned short soLuongNot, unsigned short soLuongLapLai, float thoiBatDauHat, float *thoiGian, float amLuong ) {
   
   unsigned short soNot = 0;
   float thoiGianHat = thoiBatDauHat;
   
   unsigned short soLapLai = 0;
   
   while ( soLapLai < soLuongLapLai ) {
      unsigned short chiSo = 0;
      // ----
      while( chiSo < soLuongNot ) {

         mangNot[soNot].giaiDieu = kGIAI_DIEU_5;

         if( ((rand() & 3) == 3) && soLapLai ) {  // không cần đổi nối cho lạp đầu tiên
            mangNot[soNot].doan = mangSoDoan[chiSo] + (rand() & 3) - 1;
            mangNot[soNot].so = rand() % 5;
         }
         else {
            mangNot[soNot].doan = mangSoDoan[chiSo];
            mangNot[soNot].so = mangSoNot[chiSo];
         }
         
//         printf( "chiSo %d - %d %d\n", chiSo, mangNot[soNot].doan, mangNot[soNot].so );

         mangNot[soNot].thoiHat = thoiGianHat;
         mangNot[soNot].hatBaoLau = mangHatNotBaoLau[chiSo];
         
         mangNot[soNot].amLuong = amLuong*0.5f;
         mangNot[soNot].viTriTraiTrai = 0.5f;
         mangNot[soNot].viTriTraiPhai = 1.0f;
         soNot++;
         
         mangNot[soNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soNot].doan = mangSoDoan[soNot-1] - 1;
         mangNot[soNot].so = mangSoNot[soNot-1];
         
         mangNot[soNot].thoiHat = thoiGianHat;
         mangNot[soNot].hatBaoLau = mangHatNotBaoLau[chiSo];
         
         mangNot[soNot].amLuong = amLuong*0.5f;
         mangNot[soNot].viTriTraiTrai = 1.0f;
         mangNot[soNot].viTriTraiPhai = 0.5f;
         soNot++;
         
         thoiGianHat += mangCachNot[chiSo];
         chiSo++;
      }
      soLapLai++;
   }
   *thoiGian = thoiGianHat;
   
   return soNot;
}


#pragma mark ---- NGẪU NHIÊN
// ---- chọn nốt ngẫu nhiên cho một đoàn thời gian
unsigned short datNotChoSaoLapLaiNgauNhien( Not *mangNot, float thoiBatDauHat, float *thoiGian, float amLuong,
                                           float doanThoiGian, unsigned short soLanLapLai ) {
   
   unsigned short soLuongNot = 0;
   float thoiGianHat = thoiBatDauHat;
   
   unsigned short chiSo = 0;
   // ----
   while( thoiGianHat < doanThoiGian ) {
      // ----
      
      unsigned char not = rand() % 5;
      float hatBaoLau = 0.125f;
      if( rand() & 1 )
         hatBaoLau = 0.25f;
      //      else if( rand() & 7 == 7 )
      //         hatBaoLau = 0.5f;
      
      if( not < 5 ) {
         mangNot[soLuongNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soLuongNot].doan = (rand() % 3) + 1;
         mangNot[soLuongNot].so = not;
         
         mangNot[soLuongNot].thoiHat = thoiGianHat;
         mangNot[soLuongNot].hatBaoLau = hatBaoLau;
         
         mangNot[soLuongNot].amLuong = amLuong*0.5f;
         mangNot[soLuongNot].viTriTraiTrai = 0.5f;
         mangNot[soLuongNot].viTriTraiPhai = 1.0f;
         soLuongNot++;
         
         mangNot[soLuongNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soLuongNot].doan = mangNot[soLuongNot-1].doan - 3;
         mangNot[soLuongNot].so = mangNot[soLuongNot-1].so;
         mangNot[soLuongNot].thoiHat = thoiGianHat;
         mangNot[soLuongNot].hatBaoLau = hatBaoLau;
         
         mangNot[soLuongNot].amLuong = amLuong*0.5f;
         mangNot[soLuongNot].viTriTraiTrai = 1.0f;
         mangNot[soLuongNot].viTriTraiPhai = 0.5f;
         soLuongNot++;
         thoiGianHat += mangNot[soLuongNot].hatBaoLau;
         
         chiSo++;
      }
      thoiGianHat += hatBaoLau;
   }

   unsigned short baoNhieuNot = soLuongNot;
   unsigned short soLapLai = 0;
   while( soLapLai < soLanLapLai ) {
      chiSo = 0;
      
      while( chiSo < baoNhieuNot ) {
         printf( "%d - %d %d  thoiHat %5.3f  hatBaoLau %5.3f\n", chiSo>> 1, mangNot[chiSo].doan, mangNot[chiSo].so, mangNot[chiSo].thoiHat, mangNot[chiSo].hatBaoLau );
         
         // ----
         mangNot[soLuongNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soLuongNot].doan = mangNot[chiSo].doan;
         mangNot[soLuongNot].so = mangNot[chiSo].so;
         
         mangNot[soLuongNot].thoiHat = thoiGianHat;
         mangNot[soLuongNot].hatBaoLau = mangNot[chiSo].hatBaoLau;
         
         mangNot[soLuongNot].amLuong = amLuong*0.5f;
         mangNot[soLuongNot].viTriTraiTrai = 0.5f;
         mangNot[soLuongNot].viTriTraiPhai = 1.0f;
         soLuongNot++;
         
         mangNot[soLuongNot].giaiDieu = kGIAI_DIEU_5;
         mangNot[soLuongNot].doan = mangNot[chiSo+1].doan;
         mangNot[soLuongNot].so = mangNot[chiSo+1].so;
         
         mangNot[soLuongNot].thoiHat = thoiGianHat;
         mangNot[soLuongNot].hatBaoLau = mangNot[chiSo+1].hatBaoLau;
         
         mangNot[soLuongNot].amLuong = amLuong*0.5f;
         mangNot[soLuongNot].viTriTraiTrai = 1.0f;
         mangNot[soLuongNot].viTriTraiPhai = 0.5f;
         
         thoiGianHat += mangNot[chiSo+1].hatBaoLau;
         soLuongNot++;
         
         chiSo+=2;
      }
      soLapLai++;
      
   }
   *thoiGian = thoiGianHat;
/*      printf( "-----------\n" );
    chiSo = 0;
    while( chiSo < soLuongNot ) {
    printf( "%d - %d %d  thoiHat %5.3f  hatBaoLau %5.3f\n", chiSo>> 1, mangNot[chiSo].doan, mangNot[chiSo].so, mangNot[chiSo].thoiHat, mangNot[chiSo].hatBaoLau );
    chiSo += 2;
    } */
   return soLuongNot;
}

#pragma mark ---- ÂM NHẠC TRƯỜNG 1
unsigned short datTrongPhimTruong1( Not *mangNot, float *tongThoiGian );

// ---- Đàn Dây 0
unsigned short datNotChoDanDay0_phimTruong1( Not *mangNot, float *thoiGian );
// ---- Đàn Huyên Náo 0
unsigned short datNotChoDanHuyenNao0_phimTruong1( Not *mangNot, float *thoiGian );

AmNhacTruong datAmNhacTruong1() {
   
   float thoiGianHat = 30.0f;
   
   AmNhacTruong amNhacTruong;
   // ---- dữ liệu âm nhạc
   amNhacTruong.tongThoiGianHat = thoiGianHat;
   amNhacTruong.duLieuAmThanh.soLuongKenh = 2;
   amNhacTruong.duLieuAmThanh.tocDoMauVat = hTOC_DO_MAU_VAT_MAC_DINH_48000;
   unsigned int soLuongMauVat = hTOC_DO_MAU_VAT_MAC_DINH_48000*thoiGianHat;
   amNhacTruong.duLieuAmThanh.soLuongMauVat = soLuongMauVat;  // chứng nào có nốt nhạc mới có thê tính này
   amNhacTruong.duLieuAmThanh.soLuongKenh = 2;  // nhị kênh
   amNhacTruong.duLieuAmThanh.mangMauVatKenh0 = malloc( sizeof( float )*soLuongMauVat );
   amNhacTruong.duLieuAmThanh.mangMauVatKenh1 = malloc( sizeof( float )*soLuongMauVat );
   
   amNhacTruong.soLuongLan = 2;
   amNhacTruong.danhSachLan = malloc( sizeof( Lan )*amNhacTruong.soLuongLan );
   
   // ---- làn 0 - trống
   unsigned short soLuongNot = 0;
   float thoiGian = 0.0f;
   amNhacTruong.danhSachLan[0].dan.loai = kKIEU_DAN__SONG;
   amNhacTruong.danhSachLan[0].dan.kieuDan.danSong = datDan_danDay0();
   amNhacTruong.danhSachLan[0].soLuongNot = datNotChoDanDay0_phimTruong1( amNhacTruong.danhSachLan[0].mangNot, &thoiGian );

   thoiGian = 14.0f;
   amNhacTruong.danhSachLan[1].dan.loai = kKIEU_DAN__HUYEN_NAO;
   amNhacTruong.danhSachLan[1].dan.kieuDan.danHuyenNaoTrang = datDanHuyenNao0();
   amNhacTruong.danhSachLan[1].soLuongNot = datNotChoDanHuyenNao0_phimTruong1( amNhacTruong.danhSachLan[1].mangNot, &thoiGian );

   
   printf( "soLuongNot danDay1 %d  thoiGian %5.3f\n", amNhacTruong.danhSachLan[0].soLuongNot, thoiGian );

   
   return amNhacTruong;
}


#pragma mark ---- Phim Trường 0 - Đàn Dây 0
unsigned short datNotChoDanDay0_phimTruong1( Not *mangNot, float *thoiGian ) {
   
   float tongThoiGian = 0.0f;
   unsigned short soLuongNot = 0;
   
   // ---- 
   unsigned char soDoan1[16] = {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1};
   unsigned char soDoan2[16] = {2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2};
   unsigned char soDoan3[16] = {3, 3, 3, 3,  3, 3, 3, 3,  3, 3, 3, 3,  3, 3, 3, 3};
   unsigned char soDoan4[16] = {4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4};

   unsigned char soNotBien[16] = {0, 0, 1, 2,  3, 2, 1, 0,  0, 0, 1, 2,  3, 2, 1, 0};
   float cachNotBien[16] = {0.125f, 0.125f, 0.125f, 0.125f,  0.125f, 0.125f, 0.125f, 0.125f,
      0.125f, 0.125f, 0.125f,  0.125f,  0.125f, 0.125f, 0.125f, 0.125f};
   float hatNotBaoLauBien[16] = {0.150f, 0.150f, 0.150f, 0.150f, 0.150f, 0.150f, 0.150f, 0.150f,
      0.150f, 0.150f, 0.150f,  0.150f, 0.150f, 0.150f, 0.150f, 0.150f};
   
   soLuongNot += chepLaiNot_chinhNgauNhien(&(mangNot[soLuongNot]), soDoan2, soNotBien,
                                           cachNotBien, hatNotBaoLauBien, 16, 20, tongThoiGian, &tongThoiGian, 0.2f );
//   printf( "sao  tongThoiGian %5.3f soLuongNot %d\n", tongThoiGian, soLuongNot );
   tongThoiGian = 0.0f;

   soLuongNot += chepLaiNot_chinhNgauNhien(&(mangNot[soLuongNot]), soDoan4, soNotBien,
                                           cachNotBien, hatNotBaoLauBien, 16, 20, tongThoiGian, &tongThoiGian, 0.2f );

   *thoiGian = tongThoiGian;
   soLuongNot += chepLaiNot_chinhNgauNhien(&(mangNot[soLuongNot]), soDoan1, soNotBien,
                                           cachNotBien, hatNotBaoLauBien, 16, 10, tongThoiGian, &tongThoiGian, 0.3f );
   //   printf( "sao  tongThoiGian %5.3f soLuongNot %d\n", tongThoiGian, soLuongNot );
   tongThoiGian = *thoiGian;
   soLuongNot += chepLaiNot_chinhNgauNhien(&(mangNot[soLuongNot]), soDoan3, soNotBien,
                                           cachNotBien, hatNotBaoLauBien, 16, 10, tongThoiGian, &tongThoiGian, 0.3f );
   *thoiGian = tongThoiGian;
   return soLuongNot;
}

#pragma mark ---- Phim Trường 0 - Đàn Huyên Náo 0
unsigned short datNotChoDanHuyenNao0_phimTruong1( Not *mangNot, float *thoiGian ) {
   
   float tongThoiGian = 14.0f;
   unsigned short soLuongNot = 0;
   
   // ----
   unsigned char soDoan[2] = {0, 0};
   
   unsigned char soNot[2] = {0, 0};
   float cachNotBien[2] = {0.5f, 1.0f};
   float hatNotBaoLauBien[2] = {1.0f, 1.0f};

   soLuongNot += chepLaiNot(&(mangNot[soLuongNot]), soDoan, soNot,
                                           cachNotBien, hatNotBaoLauBien, 2, 1, tongThoiGian, &tongThoiGian, 0.7f );
   //   printf( "sao  tongThoiGian %5.3f soLuongNot %d\n", tongThoiGian, soLuongNot );
   return soLuongNot;
}

#pragma mark ---- ÂM NHẠC TRƯỜNG 2

AmNhacTruong datAmNhacTruong2() {

   float thoiGianHat = 10.0f;
   
   AmNhacTruong amNhacTruong;
   // ---- dữ liệu âm nhạc
   amNhacTruong.tongThoiGianHat = thoiGianHat;
   amNhacTruong.duLieuAmThanh.soLuongKenh = 2;
   amNhacTruong.duLieuAmThanh.tocDoMauVat = hTOC_DO_MAU_VAT_MAC_DINH_48000;
   unsigned int soLuongMauVat = hTOC_DO_MAU_VAT_MAC_DINH_48000*thoiGianHat;
   amNhacTruong.duLieuAmThanh.soLuongMauVat = soLuongMauVat;  // chứng nào có nốt nhạc mới có thê tính này
   amNhacTruong.duLieuAmThanh.soLuongKenh = 2;  // nhị kênh
   amNhacTruong.duLieuAmThanh.mangMauVatKenh0 = malloc( sizeof( float )*soLuongMauVat );
   amNhacTruong.duLieuAmThanh.mangMauVatKenh1 = malloc( sizeof( float )*soLuongMauVat );
   
   amNhacTruong.soLuongLan = 1;
   amNhacTruong.danhSachLan = malloc( sizeof( Lan )*amNhacTruong.soLuongLan );
   
   // ---- làn 0 - trống
   unsigned short soLuongNot = 0;
   float thoiGian = 0.0f;
   amNhacTruong.danhSachLan[0].dan.loai = kKIEU_DAN__DOI_TAN_SO;
   amNhacTruong.danhSachLan[0].dan.kieuDan.danSong = datDan_chuongSat(); // ÂM LƯỚNG RẤT LỚN
   amNhacTruong.danhSachLan[0].soLuongNot = 0; // <----- chưa có gì cả
   //   amNhacTruong.danhSachLan[0].soLuongNot = datNotChoSaoLapLaiNgauNhien( &(amNhacTruong.danhSachLan[0].mangNot[soLuongNot]), 0.0f, &thoiGian, 0.5f, 4.0f, 4 );
   
   return amNhacTruong;
}
