//  Ví dụ phương pháp kết xuất dò tia đơn giản
//  Phiên Bản 4.25
//  Phát hành 2559/09/18
//  Hệ tọa độ giống OpenGL (+y là hướng lên)
//  Khởi đầu 2557/12/18

//  Sai lầm Segmentation fault: 11 thường nghĩa là có hai vật thể cùng tâm và kh©ông thể xây cây thần bậc

//  Biên dịch cho gcc: gcc -lm -lz doTia.c -o <tên chương trình>
//  Biên dịch cho clang: clang -lm -lz doTia.c -o <tên chương trình>
//  Lệnh dạng: <sỗ phim trường> <số hoạt hình đầu> <số hoạt hình cuối> <beRồng ảnh> <bề cao ảnh> <cỡ kích điểm ảnh>
//  ---- Cho phối cảnh
//  Lệnh dạng ví dụ: <tên chương trình biên dịch> 0  0 1520   600  300   0.01
//  Lệnh dạng ví dụ: <tên chương trình biên dịch> 0  0 1520  8192 4096  0.00075 <---- 8K  (8192 x 4320)
//  Lệnh dạng ví dụ: <tên chương trình biên dịch> 0  0 1520  4096 2048  0.0015  <---- 4K  (4096 x 2160)
//  Lệnh dạng ví dụ: <tên chương trình biên dịch> 0  0 1520  2048 1024  0.003  <---- 2K
//  Lệnh dạng ví dụ: <tên chương trình biên dịch> 0  0 1520  1536  768  0.004  <---- ~ Câu Truyện Đồ Chơi 1
//  Lệnh dạng ví dụ: <tên chương trình biên dịch> 0  0 1520  1024  512  0.006
//  Lệnh dạng ví dụ: <tên chương trình biên dịch> 0  0 1520   512  256  0.014
//  Lệnh dạng ví dụ: <tên chương trình biên dịch> 0  0 1520   256  128  0.024
//
//  • CHO MÁY PHIM 1 ĐƠN VỊ CHIẾU_TOAN_CANH ≈ 5/24 ĐƠN VỊ CHIẾU_PHỐI_CẢNH
//  • Đừng cho vật thể nào có bao bì bề nào = 0
//  - Hình Cầu
//  - Mặt Phẳng
//  - Hợp
//  - Hình Trụ
//  - Hình Nón
//  - Mặt Hyperbol
//  - Mặt Parabol
//  - Tứ Diện
//  - Bát Diện
//  - Thập Nhị Diện
//  - Kim Tư Tháp
//  - Dốc
//  - Nhị Thập Diện
//  - Sao Gai
//  - Mặt Sóng
//  - Hình Xuyến

//  - Vật Thể Bool
//  - Vật Thể Ghép

//  - Biến Hóa cho vật thể, không đặt vị trí hay phóng to ---> xài ma trận;
//       Lưu Ý: nên không xài cho phóng to lớn cho vật thể xa máy quay phim
//  - Họa tiết tô màu
//  - Phim Trường
//  - ba chiếu đồ: phối cảnh, cự tuyến, toàn cảnh
//  - phản xạ thuộc màu cho một họa tiết có thể có phản xạ không đều
//  - bóng tối trong, nhưng chỉ xài đục của vật thể đầu tiên tia bóng tối gặp

//  Hồ Nhựt Châu  su_huynh@yahoo.com

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <zlib.h>

#define kSAI  0
#define kDUNG 1

#define kKIEU_HALF 1
#define kKIEU_FLOAT 2

#define kKHONG_BIH 0

#define kVO_CUC 1.0e9f    // gía trị cho vô cực, cũng xài cho biết tia nào không trúng vật thể

#define kSO_LUONG_VAT_THE_TOI_DA 16384    // số lượng tố đa có thể chứa

#define kSO_NGUYEN_TOI_DA  2147483647.0f // <--- cho 64 bit? // 32767.0f // <--- cho 32 bit?    // cho đơn vị hóa số ngẫu nhiên

// ========= KIỂU DỮ LIỆU CƠ BẢN
/* Vectơ - cũng xài cho điểm */
typedef struct {
   float x;   // tọa đồ x; y; z
   float y;
   float z;
} Vecto;

/* Tia */
typedef struct {
   Vecto goc;       // điểm gốc
   Vecto huong;     // hướng
   Vecto daoNghichHuong;  // đạo nghịch hướng
   unsigned char dau[3];     // dấu cho hướng 
} Tia;

/* Màu */
typedef struct {
   float d;  // đỏ
   float l;  // lục
   float x;  // xanh
   float dd; // độ đục
   float c;  // cách xa
   float p;  // phản xạ
} Mau;

/* Bao Bì */
typedef struct {
   Vecto gocCucTieu;
   Vecto gocCucDai;
} BaoBi;

/* Quaternion */
typedef struct {
   float w;
   float x;
   float y;
   float z;
} Quaternion;


// =========== HỌA TIẾT
/* Họa Tiết Không */
typedef struct {
   Mau mau;     // màu
} HoaTietKhong;

/* Họa Tiết Dị Hướng */
typedef struct {
   Mau mauRanh;     // màu ranh
   Mau mauTam;      // màu tâm
} HoaTietDiHuong;

/* Họa Tiết Ca Rô */
typedef struct {
   Mau mau0;     // màu0
   Mau mau1;     // màu1
   float beRong; // bề rộng (x)
   float beCao;  // bề dài (y)
   float beDai;  // bề dài (z)
} HoaTietCaRo;

typedef struct {
   Mau mauNoi;     // màu nội
   Mau mauNgoai;   // màu ngoại
   float banKinh;  // bán kính
} HoaTietVongTron;

typedef struct {
   Mau mau0;     // màu0
   Mau mau1;     // màu1hinhdang
   float phongTo;   // phóng to
   float beRongNet; // bề rộng nét
} HoaTietOcXoay;

typedef struct {
   Mau mauTren;   // màu trên
   Mau mauDuoi;   // màu dưới
   float beCaoTren;  // bề cao trên
   float beCaoDuoi;  // bề cao dưới
} HoaTietGan;

typedef struct {
   Mau mauNen;            // màu nền
   Mau mangMau[48];       // mảng màu cho chấm bi
   float mangBanKinh[48]; // mảng bán kính cho chấm bi
   Vecto mangVecto[48];   // mảng vectơ (vị trí cho chấm bi)
} HoaTietChamBi;

typedef struct {
   Mau mauNen;            // màu nền
   Mau mangMau[48];       // mảng màu cho chấm bi
   float mangBanKinhRanh[48]; // mảng bán kính ranh cho ngôi sao, tích cỡ ngôi sao
   float mangBanKinhNgoai[48]; // mảng bán kính ranh cho ngôi sao, tích cỡ ngôi sao
   float mangBanKinhNoi[48]; // mảng bán kính ranh cho ngôi sao, tích cỡ ngôi sao
   Vecto mangVecto[48];   // mảng vectơ (vị trí cho ngôi sao trên mặt hình cầu)
   Vecto mangHuong0[48];   // mảng hướng 0 cho biết điểm trên mặt hình cầu tương đối với nan ngôi sao
   Vecto mangHuong1[48];   // mảng hướng 1 cho biết điểm trên mặt hình cầu tương đối với nan ngôi sao
   float goc;             // góc cho nan sao (để chọn số lượng nan sao)
   float nuaGoc;          // góc nữa góc giữa nan (cần cho biết dùng ranh nào của ngôi sao)
   float banKinhRanh;     // bán kính cho đường ranh giới, (đơn vị hóa tương đối với bán kính ngoài = 1,0f)
   float gocRanh;         // góc cho điềm cần nhất của rành giới, giống cho mỗi ngôi sao, không kể  
                             // tính tương đối với điểm xa = banKinhRanhNgoai tại góc = 0
} HoaTietNgoiSaoCau;

typedef struct {
   Mau mauNen;        // màu nền
   Mau mangMau[16];   // mảng màu cho chấm bi
   float tanSo;       // tân số
   float soLuongQuan; // số lượng quăn
   float beRongQuan;  // bề rộng quằn
} HoaTietQuan;

typedef struct {
   Mau mauNen;     // màu nền
   Mau mauQuan0;   // màu quằn 0
   Mau mauQuan1;   // màu quằn 1
   Mau mauQuan2;   // màu quằn 2
   float phanQuan0;     // phần số quằn 0
   float phanQuan1;     // phần số quằn 1
   float phanQuan2;     // phần số quằn 2
   float xoay;          // xoay
   float dich;          // địch xoay
   float soLapVong;     // số lặp vòng
   float doan;          // đoạn = 2π/số lặp vòng
} HoaTietQuanXoay;

typedef struct {
   Vecto huongNgang;    // hướng ngang x địa phương
   Vecto huongDoc;      // hướng dọc y địc phương, phải vuông góc với hướng ngang
   Vecto huongToi;      // hướng tới z địa phương, sẽ tính từ ngang x dọc
   Mau mauNen;     // màu nền
   Mau mauQuan0;   // màu quằn 0
   Mau mauQuan1;   // màu quằn 1
   Mau mauQuan2;   // màu quằn 2
   float phanQuan0;     // phần số quằn 0
   float phanQuan1;     // phần số quằn 1
   float phanQuan2;     // phần số quằn 2
   float tanSoNgang;    // tân số ngang
   float tanSoToi;      // tân số tới
   float bienDo;        // biên độ
   float dich;          // địch xoay
   float soLapVong;     // số lặp vòng
   float doan;          // đoạn = 2π/số lặp vòng
} HoaTietQuanSongTheoHuong;

typedef struct {
   Mau mauNen; // màu nền
   Mau mauQuan0; // màu quằn 0
   Mau mauQuan1; // màu quằn 1
   Mau mauQuan2; // màu quằn 2
   float phanQuan0; // phần số quằn 0
   float phanQuan1; // phần số quằn 1
   float phanQuan2; // phần số quằn 2
   float tanSo; // tân số
   float bienDo; // biên độ
   float dich; // địch xoay
   float soLapVong; // số lặp vòng
   float doan; // đoạn = 2π/số lặp vòng
} HoaTietQuanSongTrucZ;

typedef struct {
   Mau mauNen;     // màu nền
   Mau mauQuan0;   // màu quằn 0
   Mau mauQuan1;   // màu quằn 1
   Mau mauQuan2;   // màu quằn 2
   float phanQuan0;     // phần số quằn 0
   float phanQuan1;     // phần số quằn 1
   float phanQuan2;     // phần số quằn 2
   float tanSo;         // tân số
   float bienDo;        // biên độ
   float dich;          // địch xoay
   float soLapVong;     // số lặp vòng
   float doanVong;          // đoạn vòng = 2π/số lặp vòng
   float banKinhBatDauPhai;  // bán kính bắt đầu phai
   float banKinhKetThucPhai; // bán kính kết thúc phai
   float doanPhai;      // đoạn vòng = bán kính kết thúc phai - bán kính bắt đầu phai
} HoaTietQuanSongTiaPhai;

typedef struct {
   Mau mauNen;       // màu nền
   Mau mauSoc;       // màu sọc
   float quangSoc;    // quãng sọc
   float phanSoSoc;  // phân số tô sọc trong quãng sọc
   unsigned char trucSoc;  // trục sọc x; y; z
} HoaTietSoc;

typedef struct {
   Mau mau0;     // màu0
   Mau mau1;     // màu1
   float beRong; // bề rộng (x)
   float beCao;  // bề dài (y)
   float beDai;  // bề dài (z)
} HoaTietCaRoMin;

typedef struct {
   float banKinhBinh0; // bán kính bình phương 0
   float banKinhBinh1; // bán kính bình phương 1
   Vecto viTri0;   // vị trí 0
   Vecto viTri1;   // vị trí 1
   Mau mauNen;   // màu nền
   Mau mau0;     // màu0
   Mau mau1;     // màu1
   float beRong; // bề rộng (x)
   float beCao;  // bề dài (y)
   float beDai;  // bề dài (z)
} HoaTietHaiChamBi;

typedef struct {
   float banKinhBinhNoi;   // bán kính bình nội
   float banKinhBinhNgoai; // bán kính bình ngoại
   float banKinhBinhVanh;  // bán kính bình vành
   float banKinhVanh;      // bán kính vành
   Mau mauNen;     // màu nền
   Mau mauNoi;     // màu nội
   Mau mauNgoai;   // màu ngoại
   Mau mauVanh;    // màu cho vành ngoại

   float gocBongVong;     // góc cho một bông vòng (rad) tương đối với tâm vật thể
   float banKinhBinhBongVongNoi;   // bán kính bình cho bông vòng nọi
   float banKinhBinhBongVongNgoai; // bán kính bình cho bông vòng ngoại
   Mau mauBongVongNoi;     // màu nội
   Mau mauBongVongNgoai;   // màu ngoại
} HoaTietBongVong;

typedef struct {
   Mau mauDinhTroi;       // màu đỉnh trời
   Mau mauChanTroiTay;    // màu chân trời tây
   Mau mauChanTroiDong;   // màu chân trời đậng
   float gocXoayChanTroi; // góc xoay chân trời
} HoaTietBauTroi;

typedef union {
   HoaTietKhong hoaTietKhong;       // họa tiết không
   HoaTietDiHuong hoaTietDiHuong;   // họa tiết dị hướng
   HoaTietCaRo hoaTietCaRo;         // họa tiết ca rô
   HoaTietVongTron hoaTietVongTron; // họa tiết vòng tròn
   HoaTietOcXoay hoaTietOcXoay;     // họa tiết óc xoay
   HoaTietGan hoaTietGan;           // họa tiết gằn, gằn có thể theo trục x, y, z
   HoaTietChamBi hoaTietChamBi;     // hoạ tiết chấm bi
   HoaTietNgoiSaoCau hoaTietNgoiSaoCau; // hoạ tiết ngôi sao cho hình cầu
   HoaTietQuan hoaTietQuan;         // họa tiết quăn
   HoaTietQuanXoay hoaTietQuanXoay; // họa tiết quằn xoay
   HoaTietQuanSongTheoHuong hoaTietQuanSongTheoHuong; // họa tiết quằn sóng theo hướng (vectơ hướng)
   HoaTietQuanSongTrucZ hoaTietQuanSongTrucZ; // họa tiết quằn sóng quanh trục Z
   HoaTietQuanSongTiaPhai hoaTietQuanSongTiaPhai; // họa tiết quằn sóng tia phai (bán kính)
   HoaTietSoc hoaTietSoc;           // họa tiết sọc
   HoaTietCaRoMin hoaTietCaRoMin;   // họa tiết ca rô mịn
   HoaTietHaiChamBi hoaTietHaiChamBi;  // họa tiết hai chấm bi
   HoaTietBongVong hoaTietBongVong;   // họa tiết bông vòng (bông vòng tròn)
   HoaTietBauTroi hoaTietBauTroi;   // họa tiết bầu trời
} HoaTiet;                // họa tiết

#define kHOA_TIET__KHONG        0 // không có họa tiết, xài màu của vật thể
#define kHOA_TIET__DI_HUONG     1 // họa tiết dị hướng
#define kHOA_TIET__CA_RO        2 // họa tiết ca rô
#define kHOA_TIET__VONG_TRON    3 // họa tiết vòng tròn quanh trục Y
#define kHOA_TIET__OC_XOAY      4 // họa tiết óc xoay quanh trục Y
#define kHOA_TIET__GAN          5 // họa tiết gằn ngang
#define kHOA_TIET__CHAM_BI      6 // họa tiết chấm bi
#define kHOA_TIET__NGOI_SAO_CAU 7 // họa tiết nôi sao cầu
#define kHOA_TIET__QUAN         8 // họa tiết quăn cầu
#define kHOA_TIET__QUAN_XOAY    9 // họa tiết quăn cầu xoay
#define kHOA_TIET__QUAN_SONG_THEO_HUONG  10 // họa tiết quăn sóng theo hướng
#define kHOA_TIET__QUAN_SONG_TRUC_Z    11 // họa tiết quăn sóng theo hướng
#define kHOA_TIET__QUAN_SONG_TIA_PHAI  12 // họa tiết quăn sóng tia phai
#define kHOA_TIET__SOC         13 // họa tiết sọc
#define kHOA_TIET__CA_RO_MIN   14 // họa tiết ca rô mịn
#define kHOA_TIET__HAI_CHAM_BI 15 // họa tiết hai chấm bi
#define kHOA_TIET__BONG_VONG   16 // họa tiết bông vòng (bông vòng tròn)
#define kHOA_TIET__BAU_TROI    20 // họa tiết bầu trời
#define kHOA_TIET__TRAI_BANH   30 // họa tiết cho trái banh


// ========= HÌNH DẠNG
/* HìnhCầu */
typedef struct {
   float banKinh;
   float hopQuanh[6];   // hộp quanh
} HinhCau;

/* Hộp */
typedef struct {
   float beRong;   // bề rộng (x)
   float beCao;    // bề cao  (y)
   float beDai;    // bề dày  (z)
   float hopQuanh[6];   // hộp quanh
} Hop;

/* Mặt Phẳng */
typedef struct {
   Vecto viTri;    // vị trí
   float beRong;   // bề dài cảnh X
   float beDai;    // bề dài cảnh Z
   float hopQuanh[6];   // hộp quanh
} MatPhang;

/* Hình Trụ */
typedef struct {
   float banKinh;
   float beCao;
   float hopQuanh[6];   // hộp quanh
} HinhTru;

/* Hình Nón */
typedef struct {
   float banKinhTren;  // bán kính trên
   float banKinhDuoi;  // bán kính dưới
   float beCao;    // bề cao quanh trung tâm
   float hopQuanh[6];   // hộp quanh
} HinhNon;

/* Mat Hyperbol */
typedef struct {
   float banKinh;  // bán kính
   float cachXa;   // cách xa
   float beCao;    // bề cao quanh trung tâm
   float hopQuanh[6];   // hộp quanh
} MatHyperbol;

/* Mat Parabol */
typedef struct {
   float banKinh;  // bán kính
   float beCao;
   float hopQuanh[6];   // hộp quanh
} MatParabol;

/* Mặt Sóng */
typedef struct {
   Vecto viTri;    // vị trí
   float beRong;   // bề dài cảnh X
   float beDai;    // bề dài cảnh Z
   float bienDo0;  // biên độ 0
   float bienDo1;  // biên độ 1
   float bienDo2;  // biên độ 2
   float bienDoNhat;  // biến độ nhất = biên độ 0 + biên độ 1 + biên độ 2
   float hopQuanh[6];  // hộp quanh
} MatSong;

/* Hình Xuyến */
typedef struct {
   Vecto viTri;    // vị trí
   float banKinhVong;   // bán kính vòng
   float banKinhOng;    // bán kính ống
   float hopQuanh[6];  // hộp quanh
} HinhXuyen;

/* Tam Giác */
typedef struct {
   unsigned short dinh0;
   unsigned short dinh1;
   unsigned short dinh2;
   Vecto phapTuyen;
} TamGiac;

/* Tứ Diện */
typedef struct {
//   Vecto viTri;    // vị trí
//   float banKinh;  // bán kính
   Vecto mangDinh[4];   // mảng đỉnh
   TamGiac mangTamGiac[4]; // mảng mặt
   unsigned short soLuongTamGiac;   // số lượng tam giác
   float hopQuanh[6];   // hộp quanh
} TuDien;

/* Bát Diện */
typedef struct {
   Vecto mangDinh[6];   // mảng đỉnh
   TamGiac mangTamGiac[8];  // mảng mặt
   unsigned short soLuongTamGiac;   // số lượng tam giác
   float hopQuanh[6];   // hộp quanh
} BatDien;

/* Thập Nhị Diện */
typedef struct {
   Vecto mangDinh[32];   // mảng đỉnh
   TamGiac mangTamGiac[60];  // mảng mặt
   unsigned short soLuongTamGiac;   // số lượng tam giác
   float hopQuanh[6];   // hộp quanh
} ThapNhiDien;

/* Kim Tư Tháp */
typedef struct {
   Vecto mangDinh[5];   // mảng đỉnh
   TamGiac mangTamGiac[6];  // mảng mặt
   unsigned short soLuongTamGiac;   // số lượng tam giác
   float hopQuanh[6];   // hộp quanh
} KimTuThap;

typedef struct {
   Vecto mangDinh[6];   // mảng đỉnh
   TamGiac mangTamGiac[8];  // mảng mặt
   unsigned short soLuongTamGiac;   // số lượng tam giác
   float hopQuanh[6];   // hộp quanh
} Doc;


/* Nhi Thập Diên */
typedef struct {
   Vecto mangDinh[12];   // mảng đỉnh
   TamGiac mangTamGiac[20];  // mảng mặt
   unsigned short soLuongTamGiac;   // số lượng tam giác
   float hopQuanh[6];   // hộp quanh
} NhiThapDien;

/* Sao Gai */
typedef struct {
   Vecto mangDinh[32];   // mảng đỉnh
   TamGiac mangTamGiac[60];  // mảng mặt
   unsigned short soLuongTamGiac;   // số lượng tam giác
   float hopQuanh[6];   // hộp quanh
} SaoGai;

typedef union {
   HinhCau hinhCau;    // hình cầu
   Hop hop;            // hộp
   MatPhang matPhang;  // mặt phẳng
   HinhTru hinhTru;    // hình trụ
   HinhNon hinhNon;    // hình nón
   MatHyperbol matHyperbol; // mặt hyperbol
   MatParabol matParabol;   // mặt hyperbol
   MatSong matSong;         // mặt sóng
   HinhXuyen hinhXuyen;     // hình xuyến
   TuDien tuDien;     // tứ diện
   BatDien batDien;   // bát diện
   ThapNhiDien thapNhiDien;  // thập nhị diện
   KimTuThap kimTuThap;  // kim tư tháp
   Doc doc;              // dốc
   NhiThapDien nhiThapDien;  // nhị thập diện
   SaoGai saoGai;       //  sai gai
} HinhDang;             // hình dạng

// ---- Lọai Hình Dạng
#define kLOAI_HINH_DANG__HINH_CAU   1 // hình cầu
#define kLOAI_HINH_DANG__MAT_PHANG  2 // mặt phẩng
#define kLOAI_HINH_DANG__HOP        3 // hợp
#define kLOAI_HINH_DANG__HINH_TRU   4 // hình trụ
#define kLOAI_HINH_DANG__HINH_NON   5 // hình trụ
#define kLOAI_HINH_DANG__MAT_HYPERBOL  6 // mặt hyperbol
#define kLOAI_HINH_DANG__MAT_SONG      7
#define kLOAI_HINH_DANG__MAT_PARABOL   8 // mặt parabol
#define kLOAI_HINH_DANG__HINH_XUYEN    9 // hình xuyến
#define kLOAI_HINH_DANG__TU_DIEN      10 // tú diện
#define kLOAI_HINH_DANG__BAT_DIEN     11 // bát diện
#define kLOAI_HINH_DANG__THAP_NHI_DIEN 12 // thập nhị diện
#define kLOAI_HINH_DANG__KIM_TU_THAP  13 // kim tư tháp
#define kLOAI_HINH_DANG__DOC          14 // dốc
#define kLOAI_HINH_DANG__NHI_THAP_DIEN  15 // nhị thập diện
#define kLOAI_HINH_DANG__SAO_GAI        16 // sao gai

#define kLOAI_VAT_THE__GHEP  101    // vật thể ghép
#define kLOAI_VAT_THE__BOOL  102    // vật thể bool

#define kLOAI__BOOL_HIEU     1   // Loại bool hiệu - khắc một vật từ 
#define kLOAI__BOOL_GIAO     2   // loai bool giao - trong cả vật thể

// ========== VẬT THỂ
/* Vật Thể */
struct VatThe {
   HinhDang hinhDang;    // hìng dạng
   unsigned char loai;   // loại
   float phongTo[16];
   float xoay[16];
   float dich[16];
   float bienHoa[16];    // ma trận biến hóa

   float nghichDich[16];
   float nghichXoay[16];
   float nghichPhongTo[16];
   float nghichBienHoa[16];  // ma trận nghịch biến hóa
   float nghichBienHoaChoPhapTuyen[16];  // ma trận xài cho tính pháp tuyến (đổi thành toạ độ thế giới)
   
   // ---- cho các phim trường
   Quaternion quaternion;
   Vecto vanToc;   // vận tốc, xài khi cần giữ vận tốc từ thời trước, ví dụ rớt xuống bởi hấp dẫn
   Vecto viTriDau; // vị trí đầu, xài khi cần tính vị trí tương đối với vị trí ban đầu
   // ---- cho phim trường 0
   unsigned char duongVao; // đường vào, cho chọn đường ngẫu nhiên
   unsigned char duongRa;  // đường ra, cho chọn đường ngẫu nhiên
   unsigned char thaiTrang; // trong vòng tròn

   Mau mau;   // màu của vật
//   float tiSoPhanXa;  // tỉ số phản xạ
   float chietSuat;    // chiết suất khúc xạ
   unsigned char soHoaTiet;   // tên họa tiết màu
   HoaTiet hoaTiet;

   // ---- bao bì
   BaoBi baoBiVT; // bao bì toạ độ vật thể
   BaoBi baoBiTG; // bao bì toạ độ thế giới

   // ---- chỉ cho vật thể ghép và bool
   struct VatThe *danhSachVatThe;  // chỉ cho vật thể ghép hay bool
   char giaTri;    // -1 hay 1 cho bool
   unsigned char mucDichBool;  // giá trị mục dịch cho thao tác bool
   unsigned soLuongVatThe;   // số lượng vật thế trong ghép hay bool
   
   // ---- chỉ cho di chuyển theo cong Bezier, có thể lập lại
   float thamSoBezier;
//   unsigned short soHoatHinh;   // số hoạt hình
};

typedef struct VatThe VatThe;   // Vật Thể

/* Thông Tin Tô Màu */
typedef struct {
   VatThe *vatThe;         // vật thể
   Vecto phapTuyenTDVT;    // pháp tuyến (tọa đồ vật thể)
   Vecto diemTrungTDVT;    // điểm trúng (tọa đồ vật thể)
   float cachXa;       // cách xa
} ThongTinToMau;

/*typedef struct {
   VatThe *vatThe;     // vật thể
   Mau mauVatThe;      // màu vật thể
   HoaTiet hoaTiet;    // họa tiết
   unsigned char loaiHoaTiet;  // loại họa tiết
   
   Vecto huongTia;     // hướng tia
   Vecto phapTuyen;    // pháp tuyến
   Vecto diemTrung;    // điểm trúng
   
   float tiSoPhanXa;   // tỉ số phản xạ
   float chietSuat;    // chiết suất khúc xạ
   
   float dich;         // địch tia, xài giảm vấn đề cho vật thê xa
} ThongTinToMau; */

/* Giao Điểm */
typedef struct {
   int conTrai;   // con trái
   int conPhai;   // con phải
   unsigned char truc;  // trục
   union {
      unsigned int chiSoVatThe;
      float cat[2];  // mặt phẳng cắt
   };
} GiaoDiemBIH;

// ---- Xài 3 bit cao 
#define kTRUC_X 0x00   // 0 0 00
#define kTRUC_Y 0x01   // 0 0 01
#define kTRUC_Z 0x02   // 0 0 10
//#define kHOP    0x04   // 0 1 00
#define kLA     0x08   // 1 0 00

#define kBEN_TRAI  0
#define kBEN_PHAI  1

// =========== BEZIER
/* Bezier */   // cho vật chuyển động
typedef struct {
   Vecto diemQuanTri[4];  // danh sách điểm quản trị
//   unsigned short soLuongDiem;  // số lượng điểm
} Bezier;


/* MáyQuayPhim */
typedef struct {
   Vecto viTri;       // vị trí
   float cachManChieu;   // cách xa màn chiếu
   float xoay[16];
   Quaternion quaternion;
   
   unsigned char kieuChieu;  // kiểu chiếu ảnh

} MayQuayPhim;

enum {
   kKIEU_CHIEU__PHOI_CANH,
   kKIEU_CHIEU__CU_TUYEN,
   kKIEU_CHIEU__TOAN_CANH,
};

/* Mật Trời */
typedef struct {
   Vecto huongAnh;
   Mau mauAnh;
} MatTroi;

/* Ảnh */
typedef struct {
   unsigned short beRong;   // bề rộng
   unsigned short beCao;    // bề cao
   float coKichDiemAnh;     // cỡ kích điểm ảnh (điểm anh/đơn vị thế giới)
   float *kenhDo;      // kênh đỏ
   float *kenhLuc;     // kênh lục
   float *kenhXanh;    // kênh xanh
   float *kenhDuc;     // kênh đục
   float *kenhXa;    // kênh cách xa
} Anh;


typedef struct {
   unsigned char moTaPhimTruong[256];  // mô tả phim trường
   MayQuayPhim mayQuayPhim;            // máy quay phim
   VatThe *danhSachVatThe;             // danh sách vật thể
   unsigned short mangChiSoVatTheSapXep[kSO_LUONG_VAT_THE_TOI_DA];       // mảng chỉ số vật thể
   // xài cho được đổi thứ tự vật thể khi tạo cây tầng bậc mà không đổi vị trí trong mảngVậtThể vì chỉ số cho vật thể cần nâng cấp hoạt hình sẽ không cấp vật thể không đúng.

   unsigned short nhanVat[128];        // nhân vật (chỉ số vật thể hoạt động)
   unsigned short soLuongVatThe;       // số lượng vật thể
   unsigned short soLuongVatTheHoatHinh; // số lượng vật thể hoạt hình
   unsigned short soPhimTruong;        // số phim trường
   
   unsigned short soHoatHinhDau;     // số hoạt hình đầu, xài cho quản lý hoạt hình
   unsigned short soHoatHinhHienTai; // số hoạt hình hiện tại, xài cho quản lý hoạt hình
   unsigned short soHoatHinhCuoi;    // số hoạt hình cuối, xài cho quản lý hoạt hình
   
   unsigned char soNhoiToiDa;   // số nhồi tối đa

   BaoBi baoBi;      // bao bì cho phim trường
   MatTroi matTroi;
   HoaTietBauTroi hoaTietBauTroi;   // họa tiết bầu trời, cho tô màu trời
} PhimTruong;


// ---- vẽ và dò tia
void veAnhChieuPhoiCanh( Anh *anh, PhimTruong *phimTruong );  // vẽ ảnh chiếu phốo cảnh
void veAnhChieuCuTuyen( Anh *anh, PhimTruong *phimTruong );   // vẽ ảnh cự tuyến
void veAnhChieuToanCanh( Anh *anh, PhimTruong *phimTruong );   // vẽ ảnh toàn cầu

//void veAnhChatLuongCao( Anh *anh, PhimTruong *phimTruong );
Mau doTia( Tia *tia, PhimTruong *phimTruong, unsigned short soNhoi);  // dò tia

#pragma mark ---- XEM CẮT
void xemTiaCoTrungVatTheGanNhat( GiaoDiemBIH *nhiCayGiaoDiem, unsigned short soLuongGiaoDiem, VatThe *danhSachVatThe, unsigned short *mangChiSoVatTheSapXep, Tia *tia, ThongTinToMau *thongTinToMauVatTheGanNhat );
unsigned char xemTiaCoTrungVatTheNao( GiaoDiemBIH *nhiCayGiaoDiem, unsigned short soLuongGiaoDiem, VatThe *danhSachVatThe, unsigned short *mangChiSoVatTheSapXep, Tia *tia, ThongTinToMau *thongTinToMauBong, unsigned char *soLuongVatTheToMauBong );
unsigned char xemTiaCatMatPhangVaTruc( Tia *tia, float toaDoMatPhang, unsigned char truc, unsigned char ben );

float xemTiaCoTrungVatTrongKhongGianVat( VatThe *vatThe, Tia *tia, Vecto *phapTuyen, Vecto *diemTrungTDVT );  // xem tia có trúng vật trong không gian vật
float xemTiaCoTrungVatBoolTrongKhongGianVat( VatThe *danhSachVatTheBool, unsigned char soLuongVat, unsigned char mucDichBool, Tia *tia, Vecto *phapTuyen, Vecto *diemTrungTDVT );  // xem tia có trúng vật bool trong không gian vật
float xemTiaCoTrungVatGhepTrongKhongGianVat( VatThe *danhSachVatTheBool, unsigned char soLuongVatThe, Tia *tia, Vecto *phapTuyen, Vecto *diemTrungTDVT );
unsigned char xemDiemTrongVatThe( VatThe *vatThe, Vecto *diem );

// ---- BAO BÌ
unsigned char xemCatBaoBi( BaoBi *baoBi, Tia *tia );
BaoBi tinhBaoBiTGChoDanhSachVatThe( VatThe *danhSachVatThe, unsigned short soLuongVatThe );  // tính bao bì thế giới cho cảnh
void tinhBaoBiTGChoVatThe( VatThe *vatThe );   // tính bao bì thế giới cho vật thể
void tinhBaoBiVTChoVatTheGhep( VatThe *vatThe );   // tính bao bì cho vật thể ghép, cũng xài cho vật thể bool

float timNhoNhatCuaTamGiaTri( float so0, float so1, float so2, float so3, float so4, float so5, float so6, float so7 );
float timLonNhatCuaTamGiaTri( float so0, float so1, float so2, float so3, float so4, float so5, float so6, float so7 );


#pragma mark TẦNG BẬC
void datLaiMangChiVatThe( unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVatThe );
unsigned int chiaVatThe( VatThe *danhSachVatThe, unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVatThe, unsigned short chiSoVatThe, BaoBi *baoBi, unsigned int truc, float toaDoMatPhang );
GiaoDiemBIH chiaVatTheVaTaoGiaoDiem( VatThe *danhSachVatThe, unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVatThe, unsigned int truc, float toaDoMatPhang,
                                     unsigned short *soLuongTrai, unsigned short *soLuongPhai );
void tinhTrucVaToaMatPhangChia( BaoBi *baoBi, unsigned int *truc, float *toaDoMatPhang );
void chiaBaoBi( unsigned int truc, float matPhangChia, BaoBi *baoBi, BaoBi *baoBiTrai, BaoBi *baoBiPhai );

void xemCay( GiaoDiemBIH *cay, unsigned short soLuongGiaoDiem );
void xemVatThe( VatThe *danhSachVatThe, unsigned short soLuongVatThe );
// ---- thấy nguồn ánh sáng
unsigned char thayNguonAnhSang( VatThe *danhSachVat, unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVat, Vecto *diemTrung, Vecto *huongAnhSang, ThongTinToMau *thongTinToMauBong, unsigned char *soLuongVatTheToMauBong );


#pragma mark CÁC HÌNH DẠNG
// ---- hình cầu
HinhCau datHinhCau( float banKinh, BaoBi *baoBiVT ); // đặt hình cầu
float xemCatHinhCau( HinhCau *hinhCau, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung );   // xem hình cầu
unsigned char xemDiemTrongHinhCau( HinhCau *hinhCau, Vecto *diem );

// ---- hộp
Hop datHop( float rong, float cao, float dai, BaoBi *baoBiVT );  // đặt hộp
float xemCatHop( Hop *hop, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung );  // xem cắt hộp
unsigned char xemDiemTrongHop( Hop *hop, Vecto *diem );

// ---- mặt phẳng
MatPhang datMatPhang( Vecto *viTri, float beRong, float beDai, BaoBi *baoBiVT ); // đặt mặt phẳng
float xemCatMatPhang( MatPhang *matPhang, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung );  // xem cắt mặt phẳng

// ---- hình trụ
HinhTru datHinhTru( float banKinh, float beCao, BaoBi *baoBiVT );  // đặt hình trụ
float xemCatHinhTru( HinhTru *hinhTru, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung );  // xem cắt hình trụ
unsigned char xemDiemTrongHinhTru( HinhTru *hinhTru, Vecto *diem );

// ---- hình nón
HinhNon datHinhNon( float banKinhTren, float banKinhDuoi, float beCao, BaoBi *baoBiVT );
float xemCatHinhNon( HinhNon *hinhNon, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung );  // xem cắt hình trụ
unsigned char xemDiemTrongHinhNon( HinhNon *hinhNon, Vecto *diem );

// ---- mặt hyberbol
MatHyperbol datMatHyperbol( float banKinh, float cachXa, float beCao, BaoBi *baoBiVT ); // đặt mặt hypperbol
float xemCatMatHyperbol( MatHyperbol *matHyperbol, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung );  // xem cắt mặt hyperbol

// ---- mặt parabol
MatParabol datMatParabol( float banKinh, float beCao, BaoBi *baoBiVT ); // đặt mặt parabol
float xemCatMatParabol( MatParabol *matParabol, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung );  // xem cắt mặt parabol

// ---- mặt sóng
MatSong datMatSong( float beRong, float beDai, float bienDo0, float bienDo1, float bienDo2, BaoBi *baoBiVT ); // đặt mặt sóng
float xemCatMatSong( MatSong *matSong, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung, float thoiGian );  // xem cắt mặt sóng

// ---- hình xuyến
HinhXuyen datHinhXuyen( float banKinhVong, float banKinhOng, BaoBi *baoBiVT ); // đặt hình xuyến
float xemCatHinhXuyen( HinhXuyen *hinhXuyen, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung, float thoiGian );  // xem cắt hình xuyến


// ---- tứ diện
TuDien datTuDien( float beRong, float beCao, float beDai, BaoBi *baoBiVT ); // đặt tứ diện
// ---- bát diện
BatDien datBatDien( float beRong, float beCao, float beDai, BaoBi *baoBiVT ); // đặt bát diện
// ---- thập nhị diện
ThapNhiDien datThapNhiDien( float beRong, float beCao, float beDai, BaoBi *baoBiVT ); // đặt thập nhị diện
// ---- kim tư tháp
KimTuThap datKimTuThap( float beRong, float beCao, float beDai, BaoBi *baoBiVT ); // đặt kim tư tháp
// ---- dốc
Doc datDoc( float beRong, float beCao, float beDai, BaoBi *baoBiVT );  // đặt dốc
// ---- sao gai
NhiThapDien datNhiThapDien( float beRong, float beCao, float beDai, BaoBi *baoBiVT );
// ---- sao gai
SaoGai datSaoGai( BaoBi *baoBiVT );

float xemCatVatTheTamGiac( Vecto *mangDinh, TamGiac *mangTamGiac, unsigned short soLuongTamGiac, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung );     // xem cắt vật thể tam giác

// ---- tam giác
Vecto tinhPhapTuyenChoTamGiac( Vecto *tamGiac );
float xemCatTamGiacMT( Vecto *mangDinhTamGiac, Tia *tia );

// ---- hủy vật thể
void huyDanhSachVatThe( VatThe *danhSachVatThe, unsigned short soLuongVatThe );

#pragma mark TÔ MÀU
Mau toMauVat( VatThe *vatThe, Vecto *diemTrungTDVT, Vecto phapTuyen, Vecto huongTia );
Mau tinhMauTanXa( Mau *mauMatTroi, Mau *vat, Vecto *huongMatTroi, Vecto *phapTuyen );  // tính màu tán xạ
Mau tinhMauTroi( Vecto *huongTia );      // tính màu trời

Tia tinhTiaPhanXa( Vecto phapTuyen, Vecto diemTrung, Vecto huongTrung );   // tính tia phản xạ
Tia tinhTiaKhucXa( Vecto phapTuyen, Vecto diemTrung, Vecto huongTrung, float chietSuat );   // tính tia khúc xạ
Mau tinhCaoQuang( MatTroi *matTroi, Vecto *huongPhanXa, float mu );  // tính cao quang
Mau toSuongMu( Vecto *huongTia, Mau *mauVat, Mau *mauTroi ); // tô sươg mù

// ---- cong Bezier
Vecto tinhViTriBezier3C( Bezier *bezier, float thamSo );   // tính vị trí Bezier 3 chiều
Vecto tinhVanTocBezier3C( Bezier *bezier, float thamSo );  // tính vận tốc Bezier 3 chiều

// ---- ảnh
Anh taoAnhVoiCoKich( unsigned short beRong, unsigned short beCao, float coKichDiemAnh ); // tạo ảnh với cỡ kích
void xoaAnh( Anh *anh );  // xóa ảnh

// ---- dịch vụ vectơ
void donViHoa( Vecto *vecto );   // đơn vị hóa
Vecto tichCoHuong( Vecto *vecto0, Vecto *vecto1 );   // tích có hướng

// ---- lưu ảnh RLE
void luuAnhZIP( char *tenTep, Anh *anh, unsigned char kieuDuLieu, unsigned short thoiGianKetXuat );   // lưu ảnh ZIP

#pragma mark Họa Tiết Thủ Tục ---- HẾT XÀI TỌA ĐỘ VẬT THỂ, KHÔNG PHẢI TỌA ĐỘ THẾ GIỚI
// ---- không (màu đều)
HoaTietKhong datHoaTietKhong( Mau *mau );

// ---- dị hướng
HoaTietDiHuong datHoaTietDiHuong( Mau *mauRanh, Mau *mauTam );
Mau hoaTietDiHuong( Vecto phapThuyen, Vecto huongTia, HoaTietDiHuong *hoaTietDiHuong );

// ---- ca rô
HoaTietCaRo datHoaTietCaRo( Mau *mau0, Mau *mau1, float beRong, float beCao, float beDai );
Mau hoaTietCaRo( Vecto *viTri, HoaTietCaRo *hoaTietCao ); // họa tiết ca rô

// ---- vòng tròn
HoaTietVongTron datHoaTietVongTron( Mau *mau0, Mau *mau1, float banKinh );
Mau hoaTietVongTron( Vecto *viTri, HoaTietVongTron *hoaTietVongTron );  // họa tiết vòng tròn quanh trục y

// ---- óc xoáy
HoaTietOcXoay datHoaTietOcXoay( Mau *mau0, Mau *mau1, float beRongNet, float phongTo );
Mau hoaTietOcXoay( Vecto *viTri, HoaTietOcXoay *hoaTietOcXoay );  // hoạ tiết óc xoáy

// ---- gằn
HoaTietGan datHoaTietGan( Mau *mauTren, Mau *mauDuoi, float beCaoTren, float beCaoDuoi );
Mau hoaTietGan( float toaDo, HoaTietGan *hoaTietGan );

// ---- chấm bị
HoaTietChamBi datHoaTietChamBi( Mau *mauNen, Mau *mauThap, Mau *mauCao );
Mau hoaTietChamBi( Vecto *viTri, HoaTietChamBi *hoaTietChamBi );

// ---- ngôi sao cầu
HoaTietNgoiSaoCau datHoaTietNgoiSaoCau( Mau *mauNen, Mau *mauThap, Mau *mauCao, float banKinhNoi, float banKinhNgoai, float chenhLech,  unsigned char soLuongNan );
Mau hoaTietNgoiSaoCau( Vecto *viTri, HoaTietNgoiSaoCau *hoaTietNgoiSaoCau );
void tinhBanKinhVaGocRanh( float *banKinh, float *gocRanh , float banKinhNoi, float banKinhNgoai, float nuaGoc );

// ---- quăn
HoaTietQuan datHoaTietQuan( Mau *mauNen, Mau *mauThap, Mau *mauCao );
Mau hoaTietQuan( Vecto *viTri, HoaTietQuan *hoaTietQuan );

// ---- quằn xoay
HoaTietQuanXoay datHoaTietQuanXoay( Mau *mauNen, Mau *mauQuan0, Mau *mauQuan1, Mau *mauQuan2, float phanQuan0, float phanQuan1, float phanQuan2, float xoay, float dich, unsigned char soLapVong );
Mau hoaTietQuanXoay( Vecto *viTri, HoaTietQuanXoay *hoaTietQuanXoay );

// ---- quằn sóng theo hướng
HoaTietQuanSongTheoHuong datHoaTietQuanSongTheoHuong( Vecto *huongNgang, Vecto *huongDoc, Mau *mauNen, Mau *mauQuan0, Mau *mauQuan1, Mau *mauQuan2, float phanQuan0, float phanQuan1, float phanQuan2, float tanSoNgang, float tanSoToi, float bienDo, float dichDoan, float doan );
Mau hoaTietQuanSongTheoHuong( Vecto *viTri, HoaTietQuanSongTheoHuong *hoaTietQuanSongTheoHuong );

// ---- quằn sóng truc z
HoaTietQuanSongTrucZ datHoaTietQuanSongTrucZ( Mau *mauNen, Mau *mauQuan0, Mau *mauQuan1, Mau *mauQuan2, float phanQuan0, float phanQuan1, float phanQuan2, float tanSo, float bienDo, float dich, unsigned char soLapVong );
Mau hoaTietQuanSongTrucZ( Vecto *viTri, HoaTietQuanSongTrucZ *hoaTietQuanSongTrucZ );

// ---- quằn sóng tia phai
HoaTietQuanSongTiaPhai datHoaTietQuanSongTiaPhai( Mau *mauNen, Mau *mauQuan0, Mau *mauQuan1, Mau *mauQuan2, float phanQuan0, float phanQuan1, float phanQuan2, float tanSo, float bienDo, float dich, unsigned char soLapVong, float banKinhBatDauPhai, float banKinhKetThucPhai );
Mau hoaTietQuanSongTiaPhai( Vecto *viTri, HoaTietQuanSongTiaPhai *HoaTietQuanSongTiaPhai );

// ---- sọc
HoaTietSoc datHoaTietSoc( Mau *mauNen, Mau *mauSoc, float quangSoc, float phanSoSoc, unsigned char trucSoc );
Mau hoaTietSoc( Vecto *viTri, HoaTietSoc *hoaTietSoc );

// ---- ca rô mịn
HoaTietCaRoMin datHoaTietCaRoMin( Mau *mau0, Mau *mau1, float beRong, float beCao, float beDai );
Mau hoaTietCaRoMin( Vecto *viTri, HoaTietCaRoMin *hoaTietCao ); // họa tiết ca rô

// ---- hai chấm bi
HoaTietHaiChamBi datHoaTietHaiChamBi( Mau *mauNen, Mau *mau0, Mau *mau1, float banKinh0, float banKinh1, Vecto *viTri0, Vecto *viTri1, float beRong, float beCao, float beDai );
Mau hoaTietHaiChamBi( Vecto *viTri, HoaTietHaiChamBi *hoaTietHaiChamBi );

// ---- bông vòng
HoaTietBongVong datHoaTietBongVong( Mau *mauNen, Mau *mauNoi, Mau *mauNgoai, Mau *mauVanh, float banKinhNoi, float banKinhNgoai, float banKinhVanh, Mau *mauBongVongNoi, Mau *mauBongVongNgoai, float tiSoToBong, unsigned short soLuongBong );
Mau hoaTietBongVong( Vecto *viTri, HoaTietBongVong *hoaTietBongVong );

// ---- bầu trời
HoaTietBauTroi datHoaTietBauTroi( Mau *mauDinhToi, Mau *mauChanTroiTay, Mau *mauChanTroiDong, float gocXoayChanTroi );
Mau hoaTietBauTroi( Vecto huongTia, HoaTietBauTroi *hoaTietBauTroi );

// ---- trái banh
Mau hoaTietTraiBanh( Vecto *viTri );   // họa tiết trái banh

// ---- chọn và tô họa tiết ngẫu nhiên
void chonVaToHoatTietNgauNhien( unsigned char *soHoaTiet, HoaTiet *hoaTiet );

#pragma mark Ma Trận
// kèm thêm ở đang sau như này: [vectơ] * [phóng to] * [xoay] * [dịch]
//                      nghịch: [vectơ] * [dịch]-1 *[xoay]-1 * [phóng to]-1
void datDonVi( float *maTran );  // đặt đơn vị (ma trận)
void datPhongTo( float *maTran, float phongToX, float phongToY, float phongToZ );  // đặt phóng to
void datDich( float *maTran, float x, float y, float z ); // đặt dịch
//void daoNghich4x4_datBiet( float *maTranDaoNguoc, float *maTranGoc );    // đảo nghịch 4x4 đặc biệt

#pragma mark maTran
void latMaTran4x4( float *maTran );
Vecto nhanVectoVoiMaTran3x3( Vecto *vecto, float *maTran );  // nhân vectơ với ma trận 3 x 3; không gồm dịch
Vecto nhanVectoVoiMaTran4x4( Vecto *vecto, float *maTran );  // nhân vectơ với ma trận 4 x 4;
void nhanMaTranVoiMaTran( float *maTranKetQua, float *maTran1, float *maTran2 );  // nhân ma trận với ma trận
void dinhHuongMaTranBangVectoNhin( float *maTran, Vecto *vectoNhin );

#pragma mark Quaternion
Quaternion datQuaternionTuVectoVaGocQuay( Vecto *vecto, float gocQuay );
void quaternionQuaMaTran( Quaternion *quaternion, float *maTran );
Quaternion nhanQuaternionVoiQuaternion( Quaternion *quaternion0, Quaternion *quaternion1 );
#pragma mark Đặt biến hóa
void datBienHoaChoVat( VatThe *vatThe, Vecto *phongTo, Quaternion *xoay, Vecto *dich );  // đặt biến hóa
//void datViTriChoVat( VatThe *vatThe, Vecto *viTri );
void datXoayChoVat( VatThe *vatThe, Quaternion *xoay );
void gomBienHoaChoVat( VatThe *vatThe ); // gồm biến hóa ch vật
Vecto xoayVectoQuanhTruc( Vecto *vecto, Vecto *trucXoay, float gocXoay );  // xoay vectơ quanh trục

#pragma mark Di Chuyển
//Vecto vanTocSin( Vecto *huong, float bienDo, float soBucAnh, float soBucAnhChuKy );
Vecto viTriSin( Vecto *viTriDau, Vecto *huong, float bienDo, float soBucAnh, float soBucAnhChuKy, float *tocDo );
Vecto viTriSin2( Vecto *viTriDau, Vecto *huong, float bienDo, float soBucAnh, float soBucAnhChuKy, float *tocDo );

#pragma mark Đọc Tham Số Dòng Lệnh
void docThamSoPhimTruong( int argc, char **argv, unsigned int *soPhimTruong );
void docThamSoHoatHinh( int argc, char **argv, unsigned int *soHoatHinhDau, unsigned int *soHoatHinhCuoi );
void docThamCoKich( int argc, char **argv, unsigned int *beRong, unsigned int *beCao, float *coThuocDiemAnh );
//Vecto anhSangMatTroi;  // hướng ánh sáng mặt trời
//Mau mauMatTroi;  // hướng ánh sáng mặt trời
unsigned int thoiGian = 0;

#pragma mark CÁC PHIM TRƯỜNG
PhimTruong datPhimTruongSo0( unsigned int argc, char **argv );
void nangCapPhimTruong0( PhimTruong *phimTruong );

PhimTruong datPhimTruongSo1( unsigned int argc, char **argv );
void nangCapPhimTruong1( PhimTruong *phimTruong );

PhimTruong datPhimTruongSo2( unsigned int argc, char **argv );
void nangCapPhimTruong2( PhimTruong *phimTruong );

// ---- Biến Tòàn Cầu
#define kSO_LUONG__GIAO_DIEM_TOI_DA   114688
GiaoDiemBIH mangGiaoDiem[kSO_LUONG__GIAO_DIEM_TOI_DA];  // 28672/7 ≈ 16384 vật thể
unsigned int chiSoGiaoDiem;


int main( int argc, char **argv ) {

   PhimTruong phimTruong;
   unsigned int soPhimTruong = 0;
   docThamSoPhimTruong( argc, argv, &soPhimTruong );

   if( soPhimTruong == 0 )
      phimTruong = datPhimTruongSo0( argc, argv );
   else if( soPhimTruong == 1 )
      phimTruong = datPhimTruongSo1( argc, argv );
   else if( soPhimTruong == 2 )
      phimTruong = datPhimTruongSo2( argc, argv );
   
   // ---- giữ số phim trường
   phimTruong.soPhimTruong = soPhimTruong;

//   thoiGian = phimTruong.soHoatHinhDau;

   // ---- cỡ kích ảnh
   unsigned int beCaoAnh;// = 201;//601;  // 601
   unsigned int beRongAnh;// = beCaoAnh << 1;
   // ---- cho hình sẽ phóng to theo cỡ kích ảnh
   float coKichDiemAnh;// = 0.005f*3.0f;   // 1.0f;
   docThamCoKich( argc, argv, &beRongAnh, &beCaoAnh, &coKichDiemAnh );
   printf( "Cỡ Kích Ảnh: %d %d %5.4f\n", beRongAnh, beCaoAnh, coKichDiemAnh );


   Anh anh = taoAnhVoiCoKich( beRongAnh, beCaoAnh, coKichDiemAnh );
   printf( "PhimTrường %d  số lượng vật thể %d/%d\n", phimTruong.soPhimTruong, phimTruong.soLuongVatThe, kSO_LUONG_VAT_THE_TOI_DA );
   printf( "     bứcẢnhĐầu %d   bứcẢnhCuối %d\n", phimTruong.soHoatHinhDau, phimTruong.soHoatHinhCuoi );

   time_t thoiGianBatDauToanCau;
   time(&thoiGianBatDauToanCau);

   while( phimTruong.soHoatHinhDau < phimTruong.soHoatHinhCuoi ) {

      // ---- nâng cấp hoạt hình
      if( soPhimTruong == 0 )
         nangCapPhimTruong0( &phimTruong );
      else if( soPhimTruong == 1 )
         nangCapPhimTruong1( &phimTruong );
      else if( soPhimTruong == 2 )
         nangCapPhimTruong2( &phimTruong );


//      xemVatThe( phimTruong.danhSachVatThe, phimTruong.soLuongVatThe );
      char tenAnh[256];
      sprintf( tenAnh, "TGTB_%02d_%04d.exr", phimTruong.soPhimTruong, phimTruong.soHoatHinhDau );

      // ---- xây tầng bậc đoạn bao bì
      phimTruong.baoBi = tinhBaoBiTGChoDanhSachVatThe( phimTruong.danhSachVatThe, phimTruong.soLuongVatThe );
 
      chiSoGiaoDiem = 0;

      datLaiMangChiVatThe( phimTruong.mangChiSoVatTheSapXep, phimTruong.soLuongVatThe );
      // ==== tạo tầng bậc bao bì
      // ---- trục và tọa độ mặt phẳng đầu
      float toaDoMatPhang;
      unsigned int truc;
      tinhTrucVaToaMatPhangChia( &(phimTruong.baoBi), &truc, &toaDoMatPhang );
      // ---- tạo cây tầng bậc bao bì
      chiaVatThe( phimTruong.danhSachVatThe, phimTruong.mangChiSoVatTheSapXep, phimTruong.soLuongVatThe, 0, &(phimTruong.baoBi), truc, toaDoMatPhang );
//      xemCay( mangGiaoDiem, chiSoGiaoDiem );
      printf( "cây chiSoGiaoDiem %d\n", chiSoGiaoDiem );
      // ==== thời gian bặt đầu tính
      time_t thoiGianBatDau;
      time(&thoiGianBatDau);

      // ==== vẽ ảnh (kết xuất)
      if( phimTruong.mayQuayPhim.kieuChieu == kKIEU_CHIEU__PHOI_CANH )
         veAnhChieuPhoiCanh( &anh, &phimTruong );
      else if( phimTruong.mayQuayPhim.kieuChieu == kKIEU_CHIEU__CU_TUYEN )
         veAnhChieuCuTuyen( &anh, &phimTruong );
      else if( phimTruong.mayQuayPhim.kieuChieu == kKIEU_CHIEU__TOAN_CANH )
         veAnhChieuToanCanh( &anh, &phimTruong );
      
      // ==== thời gian xong
      time_t thoiGianKetThuc;
      time(&thoiGianKetThuc);
      time_t thoiGianKetXuat = thoiGianKetThuc - thoiGianBatDau;

      // ---- lưu ảnh
      luuAnhZIP( tenAnh, &anh, kKIEU_HALF, (unsigned short)thoiGianKetXuat );
      
      // ---- báo lưu xong ảnh nào
      printf( "-->> %s <<--  %ld giây (%5.2f phút) %d/%d\n", tenAnh, thoiGianKetXuat, (float)thoiGianKetXuat/60.0f,
             phimTruong.soHoatHinhDau, phimTruong.soHoatHinhCuoi );

      // ---- số hoạt hình hiện tại
      phimTruong.soHoatHinhHienTai++;
   }
   // ---- in ra hết thời gian
   time_t thoiGianKetThucToanCau;
   time(&thoiGianKetThucToanCau);
   printf( "Hết thời gian: %ld giây (%5.2f phút)\n", thoiGianKetThucToanCau - thoiGianBatDauToanCau, (float)(thoiGianKetThucToanCau - thoiGianBatDauToanCau)/60.0f );

   // ---- xóa danh sách vật thể
   huyDanhSachVatThe( phimTruong.danhSachVatThe, phimTruong.soLuongVatThe );

   // ---- xong công việc, xóa ảnh
   xoaAnh( &anh );
   printf( "Cỡ kích các ảnh: %d %d\n", beRongAnh, beCaoAnh );
   return 1;
}

#pragma mark ---- Vẽ Ảnh
void veAnhChieuPhoiCanh( Anh *anh, PhimTruong *phimTruong ) {
   
   MayQuayPhim *mayQuayPhim = &(phimTruong->mayQuayPhim);
   // ---- góc tia là vị trí máy quay phim
   Tia tia;
   tia.goc.x = mayQuayPhim->viTri.x;
   tia.goc.y = mayQuayPhim->viTri.y;
   tia.goc.z = mayQuayPhim->viTri.z;

//   printf( "Tia %5.3f %5.3f %5.3f\n", tia.goc.x, tia.goc.y, tia.goc.z );
   unsigned short beRong = anh->beRong;
   unsigned short beCao = anh->beCao;

//   printf( "Tia %5.3f %5.3f %5.3f\n", gocX, gocY, gocZ );
   // ---- tính vectơ hướng lên (dộc)
   Vecto huongDoc;  // hướng dọc
   huongDoc.x = mayQuayPhim->xoay[4];
   huongDoc.y = mayQuayPhim->xoay[5];
   huongDoc.z = mayQuayPhim->xoay[6];
   donViHoa( &huongDoc );

   Vecto huongNgang;  // hướng ngang
   huongNgang.x = -mayQuayPhim->xoay[0];
   huongNgang.y = -mayQuayPhim->xoay[1];
   huongNgang.z = -mayQuayPhim->xoay[2];
   donViHoa( &huongNgang );
//   printf( "huongNgang %5.3f %5.3f %5.3f\n", huongNgang.x, huongNgang.y, huongNgang.z );
//   printf( "huongDoc %5.3f %5.3f %5.3f\n\n", huongDoc.x, huongDoc.y, huongDoc.z );

   // ---- tính vectơ cho bước ngang
   Vecto buocNgang;
   buocNgang.x = huongNgang.x*anh->coKichDiemAnh;
   buocNgang.y = huongNgang.y*anh->coKichDiemAnh;
   buocNgang.z = huongNgang.z*anh->coKichDiemAnh;
   
   // ---- tính vectơ cho bước lên (dộc)
   Vecto buocDoc;  // bước dọc
   buocDoc.x = huongDoc.x*anh->coKichDiemAnh;
   buocDoc.y = huongDoc.y*anh->coKichDiemAnh;
   buocDoc.z = huongDoc.z*anh->coKichDiemAnh;
//   printf( "buocgNgang %5.3f %5.3f %5.3f\n", buocNgang.x, buocNgang.y, buocNgang.z );
//   printf( "buocDoc %5.3f %5.3f %5.3f\n\n", buocDoc.x, buocDoc.y, buocDoc.z );

   // ---- tính điểm góc ảnh (trái dưới)
   float gocX = -(beCao >> 1)*buocDoc.x -(beRong >> 1)*buocNgang.x + tia.goc.x + mayQuayPhim->cachManChieu*mayQuayPhim->xoay[8];
   float gocY = -(beCao >> 1)*buocDoc.y -(beRong >> 1)*buocNgang.y + tia.goc.y + mayQuayPhim->cachManChieu*mayQuayPhim->xoay[9];
   float gocZ = -(beCao >> 1)*buocDoc.z -(beRong >> 1)*buocNgang.z + tia.goc.z + mayQuayPhim->cachManChieu*mayQuayPhim->xoay[10];
//   printf( "goc anh %5.3f %5.3f %5.3f\n", gocX, gocY, gocZ );
//   printf( "tia.goc %5.3f %5.3f %5.3f\n", tia.goc.x, tia.goc.y, tia.goc.z );
//   exit(0);
   unsigned long int chiSoAnh = 0;
   unsigned long int chiSoAnhCuoi = beRong*beCao;   // cho biết đang kết xuất ảnh bao nhiêu
   
   unsigned short hang = 0;
   while( hang < beCao ) {

      // ---- cho biết khi đến hàng mới
      printf( "hàng %4d/%d (%d‰)   ảnh %d/%d  PT(%d)\n", hang, beCao, hang*1000/beCao, phimTruong->soHoatHinhHienTai, phimTruong->soHoatHinhCuoi, phimTruong->soPhimTruong );

      // ---- tính hướng cho tia của điểm ảnh này
      tia.huong.x = gocX + buocDoc.x*hang - tia.goc.x;
      tia.huong.y = gocY + buocDoc.y*hang - tia.goc.y;
      tia.huong.z = gocZ + buocDoc.z*hang - tia.goc.z;

      unsigned short cot = 0;
      while( cot < beRong ) {

         Mau mauDoTia = doTia( &tia, phimTruong, 0 );
         Mau mauBauTroi = hoaTietBauTroi( tia.huong, &(phimTruong->hoaTietBauTroi) );
         Mau mauCoSuongMu = toSuongMu( &(tia.huong), &mauDoTia, &mauBauTroi );

         // ---- giữ điểm ảnh kết xuất
         anh->kenhDo[chiSoAnh] = mauCoSuongMu.d;
         anh->kenhLuc[chiSoAnh] = mauCoSuongMu.l;
         anh->kenhXanh[chiSoAnh] = mauCoSuongMu.x;
         anh->kenhDuc[chiSoAnh] = 1.0f;//mauDoTia.dd;

         chiSoAnh++;

         // ---- hướng tia tiếp
         tia.huong.x += buocNgang.x;
         tia.huong.y += buocNgang.y;
         tia.huong.z += buocNgang.z;

         cot++;
      }

      hang++;
   }

}

void veAnhChieuCuTuyen( Anh *anh, PhimTruong *phimTruong ) {
   
   MayQuayPhim *mayQuayPhim = &(phimTruong->mayQuayPhim);
   
   //   printf( "Tia %5.3f %5.3f %5.3f\n", tia.goc.x, tia.goc.y, tia.goc.z );
   unsigned short beRong = anh->beRong;
   unsigned short beCao = anh->beCao;
   
   //   printf( "Tia %5.3f %5.3f %5.3f\n", gocX, gocY, gocZ );
   // ---- tính vectơ hướng lên (dộc)
   Vecto huongDoc;  // hướng dọc
   huongDoc.x = mayQuayPhim->xoay[4];
   huongDoc.y = mayQuayPhim->xoay[5];
   huongDoc.z = mayQuayPhim->xoay[6];
   donViHoa( &huongDoc );
   
   Vecto huongNgang;  // hướng ngang
   huongNgang.x = -mayQuayPhim->xoay[0];
   huongNgang.y = -mayQuayPhim->xoay[1];
   huongNgang.z = -mayQuayPhim->xoay[2];
   donViHoa( &huongNgang );
//   printf( "huongNgang %5.3f %5.3f %5.3f\n", huongNgang.x, huongNgang.y, huongNgang.z );
//   printf( "huongDoc %5.3f %5.3f %5.3f\n\n", huongDoc.x, huongDoc.y, huongDoc.z );
   
   // ---- tính vectơ cho bước ngang
   Vecto buocNgang;
   buocNgang.x = huongNgang.x*anh->coKichDiemAnh;
   buocNgang.y = huongNgang.y*anh->coKichDiemAnh;
   buocNgang.z = huongNgang.z*anh->coKichDiemAnh;
   
   // ---- tính vectơ cho bước lên (dộc)
   Vecto buocDoc;  // bước dọc
   buocDoc.x = huongDoc.x*anh->coKichDiemAnh;
   buocDoc.y = huongDoc.y*anh->coKichDiemAnh;
   buocDoc.z = huongDoc.z*anh->coKichDiemAnh;
//      printf( "buocgNgang %5.3f %5.3f %5.3f\n", buocNgang.x, buocNgang.y, buocNgang.z );
//      printf( "buocDoc %5.3f %5.3f %5.3f\n\n", buocDoc.x, buocDoc.y, buocDoc.z );

   // ---- tính điểm góc ảnh (trái dưới)
   float gocX = -(beCao >> 1)*buocDoc.x -(beRong >> 1)*buocNgang.x + mayQuayPhim->viTri.x;
   float gocY = -(beCao >> 1)*buocDoc.y -(beRong >> 1)*buocNgang.y + mayQuayPhim->viTri.y;
   float gocZ = -(beCao >> 1)*buocDoc.z -(beRong >> 1)*buocNgang.z + mayQuayPhim->viTri.z;
//      printf( "goc anh %5.3f %5.3f %5.3f\n", gocX, gocY, gocZ );
   // ---- tính hướng cho tia
   Tia tia;
   tia.huong.x = mayQuayPhim->xoay[8];
   tia.huong.y = mayQuayPhim->xoay[9];
   tia.huong.z = mayQuayPhim->xoay[10];
//   printf( "tia.huong %5.3f %5.3f %5.3f\n", tia.huong.x, tia.huong.y, tia.huong.z );
 //  exit(0);
   
   unsigned int chiSoAnh = 0;
   unsigned int chiSoAnhCuoi = beRong*beCao;  // cho biết đang kết xuất ảnh bao nhiêu
   
   unsigned short hang = 0;
   while( hang < beCao ) {
      
      // ---- cho biết khi đến hàng mới
      printf( "hàng %4d/%d (%d‰)   ảnh %d/%d  PT(%d)\n", hang, beCao, hang*1000/beCao, phimTruong->soHoatHinhHienTai, phimTruong->soHoatHinhCuoi, phimTruong->soPhimTruong );
      
      // ---- tính hướng cho tia của điểm ảnh này
      tia.goc.x = gocX + buocDoc.x*hang;
      tia.goc.y = gocY + buocDoc.y*hang;
      tia.goc.z = gocZ + buocDoc.z*hang;
      
      unsigned short cot = 0;
      while( cot < beRong ) {
         Mau mauDoTia = doTia( &tia, phimTruong, 0 );
         Mau mauBauTroi = hoaTietBauTroi( tia.huong, &(phimTruong->hoaTietBauTroi) );
         Mau mauCoSuongMu = toSuongMu( &(tia.huong), &mauDoTia, &mauBauTroi );
         
         // ---- giữ điểm ảnh kết xuất
         anh->kenhDo[chiSoAnh] = mauCoSuongMu.d;
         anh->kenhLuc[chiSoAnh] = mauCoSuongMu.l;
         anh->kenhXanh[chiSoAnh] = mauCoSuongMu.x;
         anh->kenhDuc[chiSoAnh] = 1.0f;//mauDoTia.dd;
         chiSoAnh++;
         
         // ---- hướng tia tiếp
         tia.goc.x += buocNgang.x;
         tia.goc.y += buocNgang.y;
         tia.goc.z += buocNgang.z;
         
         cot++;
      }
      
      hang++;
   }
   
}

void veAnhChieuToanCanh( Anh *anh, PhimTruong *phimTruong ) {
   
   MayQuayPhim *mayQuayPhim = &(phimTruong->mayQuayPhim);
   // ---- góc tia là vị trí máy quay phim
   Tia tia;
   tia.goc.x = mayQuayPhim->viTri.x;
   tia.goc.y = mayQuayPhim->viTri.y;
   tia.goc.z = mayQuayPhim->viTri.z;
   
   //   printf( "Tia %5.3f %5.3f %5.3f\n", tia.goc.x, tia.goc.y, tia.goc.z );
   unsigned short beRong = anh->beRong;
   unsigned short beCao = anh->beCao;
   
   //   printf( "Tia %5.3f %5.3f %5.3f\n", gocX, gocY, gocZ );
   // ---- tính vectơ hướng lên (dộc)
   Vecto huongDoc;  // hướng dọc
   huongDoc.x = mayQuayPhim->xoay[4];
   huongDoc.y = mayQuayPhim->xoay[5];
   huongDoc.z = mayQuayPhim->xoay[6];
   donViHoa( &huongDoc );
   
   Vecto huongNgang;  // hướng ngang
   huongNgang.x = -mayQuayPhim->xoay[0];
   huongNgang.y = -mayQuayPhim->xoay[1];
   huongNgang.z = -mayQuayPhim->xoay[2];
   donViHoa( &huongNgang );
   
   //   printf( "huongNgang %5.3f %5.3f %5.3f\n", huongNgang.x, huongNgang.y, huongNgang.z );
   //   printf( "huongDoc %5.3f %5.3f %5.3f\n\n", huongDoc.x, huongDoc.y, huongDoc.z );
   
   // ---- vecto nhìn (trái dưới)
   Vecto huongMayQuayPhimNhin;
   huongMayQuayPhimNhin.x = mayQuayPhim->xoay[8];
   huongMayQuayPhimNhin.y = mayQuayPhim->xoay[9];
   huongMayQuayPhimNhin.z = mayQuayPhim->xoay[10];
   
   // ---- tính góc (rad)
   float gocNgang = (beRong >> 1)*anh->coKichDiemAnh;
   float gocDoc = -(beCao >> 1)*anh->coKichDiemAnh;
   
   float gocXoayDoc = gocDoc;
   //   printf( "goc anh %5.3f %5.3f %5.3f\n", gocX, gocY, gocZ );
   //   printf( "tia.goc %5.3f %5.3f %5.3f\n", tia.goc.x, tia.goc.y, tia.goc.z );
   //   exit(0);
   unsigned int chiSoAnh = 0;
   unsigned int chiSoAnhCuoi = beRong*beCao;   // cho biết đang kết xuất ảnh bao nhiêu
   
   unsigned short hang = 0;
   while( hang < beCao ) {
      
      // ---- cho biết khi đến hàng mới
      printf( "hàng %4d/%d (%d‰)   ảnh %d/%d  PT(%d)\n", hang, beCao, hang*1000/beCao, phimTruong->soHoatHinhHienTai, phimTruong->soHoatHinhCuoi, phimTruong->soPhimTruong );
      
      float gocXoayNgang = gocNgang;
      Vecto xoayDoc = xoayVectoQuanhTruc( &huongMayQuayPhimNhin, &huongNgang, gocXoayDoc );
//      printf( "tia.goc %5.3f %5.3f %5.3f\n", xoayDoc.x, xoayDoc.y, xoayDoc.z );
//      exit(0);
      unsigned short cot = 0;
      while( cot < beRong ) {
         
         tia.huong = xoayVectoQuanhTruc( &xoayDoc, &huongDoc, gocXoayNgang );
         
         Mau mauDoTia = doTia( &tia, phimTruong, 0 );
         Mau mauBauTroi = hoaTietBauTroi( tia.huong, &(phimTruong->hoaTietBauTroi) );
         Mau mauCoSuongMu = toSuongMu( &(tia.huong), &mauDoTia, &mauBauTroi );
         
         // ---- giữ điểm ảnh kết xuất
         anh->kenhDo[chiSoAnh] = mauCoSuongMu.d;
         anh->kenhLuc[chiSoAnh] = mauCoSuongMu.l;
         anh->kenhXanh[chiSoAnh] = mauCoSuongMu.x;
         anh->kenhDuc[chiSoAnh] = 1.0f;//mauDoTia.dd;
         
         chiSoAnh++;
         
         // ---- hướng tia tiếp
         gocXoayNgang -= anh->coKichDiemAnh;
         
         cot++;
      }
      gocXoayDoc += anh->coKichDiemAnh;
      hang++;
   }
   
}

/*void veAnhChatLuongCao( Anh *anh, PhimTruong *phimTruong ) {
   
   MayQuayPhim *mayQuayPhim = &(phimTruong->mayQuayPhim);
   Tia tia;
   tia.goc.x = mayQuayPhim->viTri.x;
   tia.goc.y = mayQuayPhim->viTri.y;
   tia.goc.z = mayQuayPhim->viTri.z;
   
   //   printf( "Tia %5.3f %5.3f %5.3f\n", tia.goc.x, tia.goc.y, tia.goc.z );
   unsigned short beRong = anh->beRong;
   unsigned short beCao = anh->beCao;
   
   //   printf( "Tia %5.3f %5.3f %5.3f\n", gocX, gocY, gocZ );
   // ---- tính vectơ hướng lên (dộc)
   Vecto huongDoc;  // hướng dọc
   huongDoc.x = mayQuayPhim->xoay[4];
   huongDoc.y = mayQuayPhim->xoay[5];
   huongDoc.z = mayQuayPhim->xoay[6];
   donViHoa( &huongDoc );
   
   Vecto huongNgang;  // hướng ngang
   huongNgang.x = -mayQuayPhim->xoay[0];
   huongNgang.y = -mayQuayPhim->xoay[1];
   huongNgang.z = -mayQuayPhim->xoay[2];
   donViHoa( &huongNgang );
   //   printf( "huongNgang %5.3f %5.3f %5.3f\n", huongNgang.x, huongNgang.y, huongNgang.z );
   //   printf( "huongDoc %5.3f %5.3f %5.3f\n\n", huongDoc.x, huongDoc.y, huongDoc.z );
   
   // ---- tính vectơ cho bước ngang
   Vecto buocNgang;
   buocNgang.x = huongNgang.x*anh->coKichDiemAnh;
   buocNgang.y = huongNgang.y*anh->coKichDiemAnh;
   buocNgang.z = huongNgang.z*anh->coKichDiemAnh;
   Vecto buocNgang_25;
   buocNgang_25.x = buocNgang.x*0.25f;
   buocNgang_25.y = buocNgang.y*0.25f;
   buocNgang_25.z = buocNgang.z*0.25f;
   
   // ---- tính vectơ cho bước lên (dộc)
   Vecto buocDoc;  // bước dọc
   buocDoc.x = huongDoc.x*anh->coKichDiemAnh;
   buocDoc.y = huongDoc.y*anh->coKichDiemAnh;
   buocDoc.z = huongDoc.z*anh->coKichDiemAnh;
   Vecto buocDoc_25;
   buocDoc_25.x = buocDoc.x*0.25f;
   buocDoc_25.y = buocDoc.y*0.25f;
   buocDoc_25.z = buocDoc.z*0.25f;
   //   printf( "buocgNgang %5.3f %5.3f %5.3f\n", buocNgang.x, buocNgang.y, buocNgang.z );
   //   printf( "buocDoc %5.3f %5.3f %5.3f\n\n", buocDoc.x, buocDoc.y, buocDoc.z );
   
   // ---- tính điểm góc ảnh (trái dưới)
   float gocX = -(beCao >> 1)*buocDoc.x -(beRong >> 1)*buocNgang.x + tia.goc.x + mayQuayPhim->cachManChieu*mayQuayPhim->xoay[8];
   float gocY = -(beCao >> 1)*buocDoc.y -(beRong >> 1)*buocNgang.y + tia.goc.y + mayQuayPhim->cachManChieu*mayQuayPhim->xoay[9];
   float gocZ = -(beCao >> 1)*buocDoc.z -(beRong >> 1)*buocNgang.z + tia.goc.z + mayQuayPhim->cachManChieu*mayQuayPhim->xoay[10];
   //   printf( "goc anh %5.3f %5.3f %5.3f\n", gocX, gocY, gocZ );
   //   printf( "tia.goc %5.3f %5.3f %5.3f\n", tia.goc.x, tia.goc.y, tia.goc.z );
   //   exit(0);
   unsigned int chiSoAnh = 0;
   
   unsigned short hang = 0;
   while( hang < beCao ) {
      
      // ---- tính hướng cho tia của điểm ảnh này
      tia.huong.x = gocX + buocDoc.x*hang - tia.goc.x;
      tia.huong.y = gocY + buocDoc.y*hang - tia.goc.y;
      tia.huong.z = gocZ + buocDoc.z*hang - tia.goc.z;
      
      unsigned short cot = 0;
      while( cot < beRong ) {

         Tia tiaPhu;
         tiaPhu.goc = tia.goc;
         // -----
         tiaPhu.huong.x = tia.huong.x - 0.25f*buocNgang.x - 0.25f*buocDoc.x;
         tiaPhu.huong.y = tia.huong.y - 0.25f*buocNgang.y - 0.25f*buocDoc.y;
         tiaPhu.huong.z = tia.huong.z - 0.25f*buocNgang.z - 0.25f*buocDoc.z;
         Mau mauDoTia = doTia( &tiaPhu, phimTruong, 0 );
         Mau mauBauTroi = hoaTietBauTroi( tiaPhu.huong, &(phimTruong->hoaTietBauTroi) );
         Mau mauCoSuongMu0 = toSuongMu( &(tiaPhu.huong), &mauDoTia, &mauBauTroi );
         
         // ----
         tiaPhu.huong.x = tia.huong.x + 0.25f*buocNgang.x - 0.25f*buocDoc.x;
         tiaPhu.huong.y = tia.huong.y + 0.25f*buocNgang.y - 0.25f*buocDoc.y;
         tiaPhu.huong.z = tia.huong.z + 0.25f*buocNgang.z - 0.25f*buocDoc.z;
         mauDoTia = doTia( &tiaPhu, phimTruong, 0 );
         mauBauTroi = hoaTietBauTroi( tiaPhu.huong, &(phimTruong->hoaTietBauTroi) );
         Mau mauCoSuongMu1 = toSuongMu( &(tiaPhu.huong), &mauDoTia, &mauBauTroi );

         // ----
         tiaPhu.huong.x = tia.huong.x + 0.25f*buocNgang.x - 0.25f*buocDoc.x;
         tiaPhu.huong.y = tia.huong.y + 0.25f*buocNgang.y - 0.25f*buocDoc.y;
         tiaPhu.huong.z = tia.huong.z + 0.25f*buocNgang.z - 0.25f*buocDoc.z;
         mauDoTia = doTia( &tiaPhu, phimTruong, 0 );
         mauBauTroi = hoaTietBauTroi( tiaPhu.huong, &(phimTruong->hoaTietBauTroi) );
         Mau mauCoSuongMu2 = toSuongMu( &(tiaPhu.huong), &mauDoTia, &mauBauTroi );
         
         // ----
         tiaPhu.huong.x = tia.huong.x + 0.25f*buocNgang.x + 0.25f*buocDoc.x;
         tiaPhu.huong.y = tia.huong.y + 0.25f*buocNgang.y + 0.25f*buocDoc.y;
         tiaPhu.huong.z = tia.huong.z + 0.25f*buocNgang.z + 0.25f*buocDoc.z;
         mauDoTia = doTia( &tiaPhu, phimTruong, 0 );
         mauBauTroi = hoaTietBauTroi( tiaPhu.huong, &(phimTruong->hoaTietBauTroi) );
         Mau mauCoSuongMu3 = toSuongMu( &(tiaPhu.huong), &mauDoTia, &mauBauTroi );
         
         // ---- giữ điểm ảnh kết xuất
         anh->kenhDo[chiSoAnh] = 0.25f*(mauCoSuongMu0.d + mauCoSuongMu1.d + mauCoSuongMu2.d + mauCoSuongMu3.d);
         anh->kenhLuc[chiSoAnh] = 0.25f*(mauCoSuongMu0.l + mauCoSuongMu1.l + mauCoSuongMu2.l + mauCoSuongMu3.l);
         anh->kenhXanh[chiSoAnh] = 0.25f*(mauCoSuongMu0.x + mauCoSuongMu1.x + mauCoSuongMu2.x + mauCoSuongMu3.x);
         anh->kenhDuc[chiSoAnh] = 1.0f;//mauDoTia.dd;
         
         chiSoAnh++;
         
         // ---- hướng tia tiếp
         tia.huong.x += buocNgang.x;
         tia.huong.y += buocNgang.y;
         tia.huong.z += buocNgang.z;
         
         cot++;
      }
      
      hang++;
   }
}*/


#define kSO_LUONG__VAT_THE_BONG_TOI_DA 16 

Mau doTia( Tia *tia, PhimTruong *phimTruong, unsigned short soNhoi ) {
   
   Mau mau;
   mau.d = 0.0f;
   mau.l = 0.0f;
   mau.x = 0.0f;
   mau.dd = 1.0f;
   mau.c = kVO_CUC;

   float cachXaGanNhat = kVO_CUC;   // ---- đặt cáchXaGầnNhất = kVÔ_HẠN

//   Vecto phapTuyenGanNhat;
//   phapTuyenGanNhat.x = 0.0f;
//   phapTuyenGanNhat.y = 0.0f;
//   phapTuyenGanNhat.z = 0.0f;

   // ==== xem nếu tia có trúng vật thể nào
   unsigned short chiSoVat = 0; // chỉ số vật thể
   ThongTinToMau thongTinToMauVatTheGanNhat;
   thongTinToMauVatTheGanNhat.vatThe = NULL;  // chưa có vật thể gần nhất

   xemTiaCoTrungVatTheGanNhat( mangGiaoDiem, chiSoGiaoDiem, phimTruong->danhSachVatThe, phimTruong->mangChiSoVatTheSapXep, tia, &thongTinToMauVatTheGanNhat );

   // ==== tính màu
   if( thongTinToMauVatTheGanNhat.vatThe != NULL ) {  // tô màu

      donViHoa( &(thongTinToMauVatTheGanNhat.phapTuyenTDVT) );
      // ---- điểm trúng (tọa độ thế giới)
      Vecto diemTrung;
      diemTrung.x = tia->goc.x + thongTinToMauVatTheGanNhat.cachXa*tia->huong.x;
      diemTrung.y = tia->goc.y + thongTinToMauVatTheGanNhat.cachXa*tia->huong.y;
      diemTrung.z = tia->goc.z + thongTinToMauVatTheGanNhat.cachXa*tia->huong.z;

      // ---- tính cách xa
      if( soNhoi == 0 ) {
         // ---- tính điểm trúng tương đối với máy quay phim
         Vecto viTriMayQuayPhim = phimTruong->mayQuayPhim.viTri;
         Vecto diemTrungTuongDoi;
         diemTrungTuongDoi.x = diemTrung.x - viTriMayQuayPhim.x;
         diemTrungTuongDoi.y = diemTrung.y - viTriMayQuayPhim.y;
         diemTrungTuongDoi.z = diemTrung.z - viTriMayQuayPhim.z;
//         printf( "doTia: %5.3f %5.3f %5.3f\n", diemTrungTuongDoi.x, diemTrungTuongDoi.y, diemTrungTuongDoi.z );
         mau.c = sqrtf( diemTrungTuongDoi.x*diemTrungTuongDoi.x + diemTrungTuongDoi.y*diemTrungTuongDoi.y + diemTrungTuongDoi.z*diemTrungTuongDoi.z );
      }
      
      // ---- điểm trúng cho nguồn ánh sáng gần nhất. Cần cộng thêm pháp tuyến môt chút cho chắc chắn ở ngoài vật thể
      Vecto diemTrungChoThayNAS; // điểm trúng cho thấy nguồn ánh sáng
      diemTrungChoThayNAS.x = diemTrung.x + 0.005f*thongTinToMauVatTheGanNhat.phapTuyenTDVT.x;
      diemTrungChoThayNAS.y = diemTrung.y + 0.005f*thongTinToMauVatTheGanNhat.phapTuyenTDVT.y;
      diemTrungChoThayNAS.z = diemTrung.z + 0.005f*thongTinToMauVatTheGanNhat.phapTuyenTDVT.z;

//      printf( "diemTrungChoThayNAS %5.3f %5.3f %5.3f\n", diemTrungChoThayNAS.x, diemTrungChoThayNAS.y, diemTrungChoThayNAS.z );
      // ---- xem nếu được thấy nguồn ánh sáng cho bóng tối (chỉ có mặt trời cách xa vô cực)
      ThongTinToMau thongTinToMauBong[kSO_LUONG__VAT_THE_BONG_TOI_DA];
      unsigned char soLuongVatTheBongToi = 0;
      unsigned char thay = thayNguonAnhSang( phimTruong->danhSachVatThe, phimTruong->mangChiSoVatTheSapXep, phimTruong->soLuongVatThe, &diemTrungChoThayNAS, &(phimTruong->matTroi.huongAnh), thongTinToMauBong, &soLuongVatTheBongToi );

      // ---- tính màu nguồn ánh sáng
      Mau mauNguonAnhSang;
      if( thay ) {
         mauNguonAnhSang.d = phimTruong->matTroi.mauAnh.d;
         mauNguonAnhSang.l = phimTruong->matTroi.mauAnh.l;
         mauNguonAnhSang.x = phimTruong->matTroi.mauAnh.x;
         mauNguonAnhSang.dd = phimTruong->matTroi.mauAnh.dd;
      }
      else {
//         printf( "soLuongVatTheBongToi %d\n", soLuongVatTheBongToi );
         // ---- tịnh độ đục từ hết vật thể
         float duc = 0.0f;
         unsigned char soVatTheBongToi = 0;
         while( soVatTheBongToi < soLuongVatTheBongToi ) {
            //         printf("thongTinToMauBong loaiVatThe %d\n", thongTinToMauBong.vatThe->loai );
            // ---- tìm đục vật thể
            Vecto toaDoHoaTiet = nhanVectoVoiMaTran3x3( &(thongTinToMauBong[soVatTheBongToi].diemTrungTDVT), thongTinToMauBong[soVatTheBongToi].vatThe->phongTo ); // <---- TẠI SAO .phongTo??? phóng to họa tiết
            // toMauVat( VatThe *vatThe, Vecto *diemTrungTDVT, Vecto phapTuyen, Vecto huongTia )
            Mau mauVat = toMauVat( thongTinToMauBong[soVatTheBongToi].vatThe, &toaDoHoaTiet, thongTinToMauBong[soVatTheBongToi].phapTuyenTDVT, phimTruong->matTroi.huongAnh );
//            printf( "mauVat.dd %5.3f  duc %5.3f\n", mauVat.dd, duc );
            if( mauVat.dd > duc )
              duc = mauVat.dd;
            soVatTheBongToi++;
         }
 /*        if( (mauVat.dd < 0.0f) || (mauVat.dd > 1.0f) ) {
            printf( "doTia: thongTinToMau.phapTuyen %5.3f %5.3f %5.3f\n", thongTinToMauBong.phapTuyenTDVT.x, thongTinToMauBong.phapTuyenTDVT.y, thongTinToMauBong.phapTuyenTDVT.z );
            printf( "doTia: thongTinToMauBong.vatThe %d  mauVat.dd %5.3f\n", thongTinToMauBong.vatThe->loai, mauVat.dd );
            exit(0);
         }*/
            
         // ---- vật thể đục
         if( duc == 1.0f ) {
            mauNguonAnhSang.d = 0.1f;  // màu ánh sánh phản xạ từ môi trường
            mauNguonAnhSang.l = 0.1f;
            mauNguonAnhSang.x = 0.2f;
            mauNguonAnhSang.dd = 1.0f;
         }
         else {
            float nghichDuc = 1.0f - duc;

            mauNguonAnhSang.d = 0.1f + 0.9f*(phimTruong->matTroi.mauAnh.d*nghichDuc);
            mauNguonAnhSang.l = 0.1f + 0.9f*(phimTruong->matTroi.mauAnh.l*nghichDuc);
            mauNguonAnhSang.x = 0.2f + 0.8f*(phimTruong->matTroi.mauAnh.x*nghichDuc);
            mauNguonAnhSang.dd = 1.0f;
         }
        
      }

      // ---- kiếm màu cho vật thể
      Vecto toaDoHoaTiet = nhanVectoVoiMaTran3x3( &(thongTinToMauVatTheGanNhat.diemTrungTDVT), thongTinToMauVatTheGanNhat.vatThe->phongTo ); // <---- TẠI SAO .phongTo???

      Mau mauVat = toMauVat( thongTinToMauVatTheGanNhat.vatThe, &toaDoHoaTiet, thongTinToMauVatTheGanNhat.phapTuyenTDVT, tia->huong );
      Mau mauTanXa = tinhMauTanXa( &mauNguonAnhSang, &mauVat, &(phimTruong->matTroi.huongAnh), &(thongTinToMauVatTheGanNhat.phapTuyenTDVT) );

      if( soNhoi + 1 < phimTruong->soNhoiToiDa ) {   // dưới giới hạn nhồi chưa
         float tiSoPhanXa = mauVat.p;
         float doDuc = mauVat.dd;
         
         Mau mauPhanXa;
         Mau mauCaoQuang;
         if( tiSoPhanXa ) {
            // ---- tia phản xạ
            Tia tiaPhanXa = tinhTiaPhanXa( thongTinToMauVatTheGanNhat.phapTuyenTDVT, diemTrung, tia->huong );
            mauPhanXa = doTia( &tiaPhanXa, phimTruong, soNhoi + 1 );
            // ---- tính màu cao quang
            if( thay || (mauVat.dd < 1.0f) ) {
               mauCaoQuang = tinhCaoQuang( &(phimTruong->matTroi), &(tiaPhanXa.huong), 250.0f*tiSoPhanXa );
               float chinh = sqrtf(tiSoPhanXa);
               mauCaoQuang.d *= chinh;  // màu ánh sánh phản xạ từ môi trường
               mauCaoQuang.l *= chinh;
               mauCaoQuang.x *= chinh;
             //  mauCaoQuang.dd = 1.0f;
            }
            else {
               mauCaoQuang.d = 0.0f;  // màu ánh sánh phản xạ từ môi trường
               mauCaoQuang.l = 0.0f;
               mauCaoQuang.x = 0.0f;
               mauCaoQuang.dd = 1.0f;
            }
         }
         else {
            mauPhanXa.d = 0.0f;
            mauPhanXa.l = 0.0f;
            mauPhanXa.x = 0.0f;
            mauPhanXa.dd = 0.0f;
            mauCaoQuang.d = 0.0f;
            mauCaoQuang.l = 0.0f;
            mauCaoQuang.x = 0.0f;
            mauCaoQuang.dd = 0.0f;
         }

         Mau mauKhucXa;
         // ---- khúc xạ
         if( doDuc < 1.0f ) {
            // ---- tia khúc xạ
            Tia tiaKhucXa = tinhTiaKhucXa( thongTinToMauVatTheGanNhat.phapTuyenTDVT, diemTrung, tia->huong, thongTinToMauVatTheGanNhat.vatThe->chietSuat );
            mauKhucXa = doTia( &tiaKhucXa, phimTruong, soNhoi + 1 );
         }
         else {
            mauKhucXa.d = 0.0f;
            mauKhucXa.l = 0.0f;
            mauKhucXa.x = 0.0f;
            mauKhucXa.dd = 0.0f;
         }

         // ---- gồm màu
         if( doDuc == 1.0f ) {
            if( tiSoPhanXa == 0.0f ) {
               mau.d = mauTanXa.d;
               mau.l = mauTanXa.l;
               mau.x = mauTanXa.x;
               mau.dd = mauTanXa.dd;
            }
            else {
               float nghichTiSoPhanXa = 1.0f - tiSoPhanXa;
               mau.d = nghichTiSoPhanXa*mauTanXa.d + tiSoPhanXa*mauPhanXa.d + mauCaoQuang.d;
               mau.l = nghichTiSoPhanXa*mauTanXa.l + tiSoPhanXa*mauPhanXa.l + mauCaoQuang.l;
               mau.x = nghichTiSoPhanXa*mauTanXa.x + tiSoPhanXa*mauPhanXa.x + mauCaoQuang.x;
               mau.dd = nghichTiSoPhanXa*mauTanXa.dd + tiSoPhanXa*mauPhanXa.dd + mauCaoQuang.dd;
            }
         }
         else {
            float nghichDoDuc = 1.0f - doDuc;
            if( tiSoPhanXa == 0.0f ) {
               mau.d = doDuc*mauTanXa.d + nghichDoDuc*mauKhucXa.d;
               mau.l = doDuc*mauTanXa.l + nghichDoDuc*mauKhucXa.l;
               mau.x = doDuc*mauTanXa.x + nghichDoDuc*mauKhucXa.x;
               mau.dd = doDuc*mauTanXa.dd + nghichDoDuc*mauKhucXa.dd;
            }
            else {
               float nghichTiSoPhanXa = 1.0f - tiSoPhanXa;
               mau.d = nghichTiSoPhanXa*(doDuc*mauTanXa.d + nghichDoDuc*mauKhucXa.d) + tiSoPhanXa*mauPhanXa.d + mauCaoQuang.d;
               mau.l = nghichTiSoPhanXa*(doDuc*mauTanXa.l + nghichDoDuc*mauKhucXa.l) + tiSoPhanXa*mauPhanXa.l + mauCaoQuang.l;
               mau.x = nghichTiSoPhanXa*(doDuc*mauTanXa.x + nghichDoDuc*mauKhucXa.x) + tiSoPhanXa*mauPhanXa.x + mauCaoQuang.x;
               mau.dd = nghichTiSoPhanXa*(doDuc*mauTanXa.dd + nghichDoDuc*mauKhucXa.dd) + tiSoPhanXa*mauPhanXa.dd + mauCaoQuang.dd;
            }
         }

      }
      else {  // đến giới hạn rồi, cho màu tán xạ
         // ---- xài màu tán xạ
         mau.d = mauTanXa.d;
         mau.l = mauTanXa.l;
         mau.x = mauTanXa.x;
         mau.dd = mauTanXa.dd;
      }
   }
   else {
      // ---- tính màu bầu trời
      mau = hoaTietBauTroi( tia->huong, &(phimTruong->hoaTietBauTroi) );
      // ---- đặt cách xa
      if( soNhoi == 0 )
         mau.c = kVO_CUC;
   }

   return mau;
}

#pragma mark ---- Xem Tia Có Trúng Vật
#define kXEP_DONG_CUC_DAI 1024
void xemTiaCoTrungVatTheGanNhat( GiaoDiemBIH *nhiCayGiaoDiem, unsigned short soLuongGiaoDiem, VatThe *danhSachVatThe, unsigned short *mangChiSoVatTheSapXep, Tia *tia, ThongTinToMau *thongTinToMauVatTheGanNhat ) {
   
   GiaoDiemBIH xepDong[kXEP_DONG_CUC_DAI];   // xếp đống
   unsigned short chiSoXepDong = 0;
   float cachXaGanNhat = kVO_CUC;
//   printf( "  cay: tia %5.3f %5.3f %5.3f   %5.3f %5.3f %5.3f\n", tia->goc.x, tia->goc.y, tia->goc.z, tia->huong.x, tia->huong.y, tia->huong.z );
   // ---- nếu có giao điểm trong cây
   if( soLuongGiaoDiem > 0 ) {
      xepDong[0] = nhiCayGiaoDiem[0];
      chiSoXepDong = 1;

      while( chiSoXepDong > 0 ) {
         chiSoXepDong--;
         GiaoDiemBIH giaoDiem = xepDong[chiSoXepDong];
         unsigned char truc = giaoDiem.truc;
//         printf( "  truc %x  conTrai %d  conPhai %d\n", truc, giaoDiem.conTrai, giaoDiem.conPhai );

         // ---- xem không phải là lá
         if( truc != kLA ) {

            if( giaoDiem.conTrai != -1 ) {
               // ---- xem cắt mặt phẳng
               float toaDoMatPhang = giaoDiem.cat[0];
               unsigned char cat = xemTiaCatMatPhangVaTruc( tia, toaDoMatPhang, truc, kBEN_TRAI ); // toạ độ thế giới

               // ---- nếu cắt, bỏ con lên xếp đống
               if( cat ) {
                  xepDong[chiSoXepDong] = nhiCayGiaoDiem[giaoDiem.conTrai];
                  chiSoXepDong++;
               }
            }

            if( giaoDiem.conPhai != -1 ) {
               // ---- xem cắt mặt phẳng
               float toaDoMatPhang = giaoDiem.cat[1];
               unsigned char cat = xemTiaCatMatPhangVaTruc( tia, toaDoMatPhang, truc, kBEN_PHAI ); // toạ độ thế giới
               // ---- nếu cắt, bỏ con lên xếp đống
               if( cat ) {
                  xepDong[chiSoXepDong] = nhiCayGiaoDiem[giaoDiem.conPhai];
                  chiSoXepDong++;
               }
            }
            
            if( chiSoXepDong >= kXEP_DONG_CUC_DAI ) {
               printf( "xemTiaCoTrungVatTheGanNhat: SAI LẦM chiSoXepDong %d ra ngoài xếp đống: %d\n", chiSoXepDong, kXEP_DONG_CUC_DAI );
               exit(0);
            }

         }
         else {  // giao điểm lá
            VatThe *vatThe = &(danhSachVatThe[mangChiSoVatTheSapXep[giaoDiem.chiSoVatThe]]);
            Vecto diemTrungTDVT;
            Vecto phapTuyenTDVT;
            Tia tiaBienHoa;
            
            // ---- biến hóa tia đến không gian vật thể  [gốc]•[dịch]-1 •[xoay]-1 •[phóng to]-1
            tiaBienHoa.goc = nhanVectoVoiMaTran4x4( &(tia->goc), vatThe->nghichBienHoa );
            // ---- đừng dịch hướng, chỉ [hướng]•[xoay]-1 •[phóng to]-1
            tiaBienHoa.huong = nhanVectoVoiMaTran3x3( &(tia->huong), vatThe->nghichBienHoa );

            // ---- xem nếu tia có trúng vật này
            float cachXa = kVO_CUC;
            cachXa = xemTiaCoTrungVatTrongKhongGianVat( vatThe, &tiaBienHoa, &phapTuyenTDVT, &diemTrungTDVT );

            // ---- xem nếu có gần hơn vật thể trước
            if( cachXa < cachXaGanNhat ) {
               thongTinToMauVatTheGanNhat->vatThe = vatThe;
               thongTinToMauVatTheGanNhat->cachXa = cachXa;
               cachXaGanNhat = cachXa;
               // ---- biến hóa pháp tuyến trờ lại tọa độ thế giới [pháp tuyến]•[phóng to]-1 •[xoay]
               thongTinToMauVatTheGanNhat->phapTuyenTDVT = phapTuyenTDVT;
               thongTinToMauVatTheGanNhat->diemTrungTDVT = diemTrungTDVT;
            }
         }
      }
   }
   else {
      thongTinToMauVatTheGanNhat->vatThe = NULL;
      thongTinToMauVatTheGanNhat->cachXa = kVO_CUC;
//      printf( "xemTiaCóTrúngVậtThểGầnNhất: SAI LẦM số lượng giao điểm == 0\n" );
      // không có vật thể, tia không trúng vật thể nào
      cachXaGanNhat = kVO_CUC;
   }

//   return cachXaGanNhat;
}

unsigned char xemTiaCoTrungVatTheNao( GiaoDiemBIH *nhiCayGiaoDiem, unsigned short soLuongGiaoDiem, VatThe *danhSachVatThe, unsigned short *mangChiSoVatTheSapXep, Tia *tia, ThongTinToMau *thongTinToMauBong, unsigned char *soLuongVatTheToMauBong ) {
   
   GiaoDiemBIH xepDong[256];   // xếp đống
   unsigned short chiSoXepDong = 0;
   float cachXaGanNhat = kVO_CUC;

   thongTinToMauBong->vatThe = NULL;
   //   printf( "  cay: tia %5.3f %5.3f %5.3f   %5.3f %5.3f %5.3f\n", tia->goc.x, tia->goc.y, tia->goc.z, tia->huong.x, tia->huong.y, tia->huong.z );
   // ---- nếu có giao điểm trong cây
   if( soLuongGiaoDiem > 0 ) {
      xepDong[0] = nhiCayGiaoDiem[0];
      chiSoXepDong = 1;
      
      while( chiSoXepDong > 0 ) {
         chiSoXepDong--;
         GiaoDiemBIH giaoDiem = xepDong[chiSoXepDong];
         unsigned char truc = giaoDiem.truc;
         //         printf( "  truc %x  conTrai %d  conPhai %d\n", truc, giaoDiem.conTrai, giaoDiem.conPhai );
         
         // ---- xem có
         if( truc != kLA ) {
            
            if( giaoDiem.conTrai != -1 ) {
               // ---- xem cắt mặt phẳng
               float toaDoMatPhang = giaoDiem.cat[0];
               unsigned char cat = xemTiaCatMatPhangVaTruc( tia, toaDoMatPhang, truc, kBEN_TRAI ); // toạ độ thế giới
               
               // ---- nếu cắt, bỏ con lên xếp đống
               if( cat ) {
                  xepDong[chiSoXepDong] = nhiCayGiaoDiem[giaoDiem.conTrai];
                  chiSoXepDong++;
               }
               
            }

            if( giaoDiem.conPhai != -1 ) {
               // ---- xem cắt mặt phẳng
               float toaDoMatPhang = giaoDiem.cat[1];
               unsigned char cat = xemTiaCatMatPhangVaTruc( tia, toaDoMatPhang, truc, kBEN_PHAI ); // toạ độ thế giới
               // ---- nếu cắt, bỏ con lên xếp đống
               if( cat ) {
                  xepDong[chiSoXepDong] = nhiCayGiaoDiem[giaoDiem.conPhai];
                  chiSoXepDong++;
               }
            }
            //            printf( " cay: chiSoXepDong %d\n", chiSoXepDong );
            
         }
         else {  // giao điểm lá
            VatThe *vatThe = &(danhSachVatThe[mangChiSoVatTheSapXep[giaoDiem.chiSoVatThe]]);
            Vecto diemTrungTDVT;
            Vecto phapTuyenTDVT;
            Tia tiaBienHoa;
            
            // ---- biến hóa tia đến không gian vật thể  [gốc]•[dịch]-1 •[xoay]-1 •[phóng to]-1
            tiaBienHoa.goc = nhanVectoVoiMaTran4x4( &(tia->goc), vatThe->nghichBienHoa );
            // ---- đừng dịch hướng, chỉ [hướng]•[xoay]-1 •[phóng to]-1
            tiaBienHoa.huong = nhanVectoVoiMaTran3x3( &(tia->huong), vatThe->nghichBienHoa );
            
            // ---- xem nếu tia có trúng vật này
            float cachXa = kVO_CUC;
            cachXa = xemTiaCoTrungVatTrongKhongGianVat( vatThe, &tiaBienHoa, &phapTuyenTDVT, &diemTrungTDVT );
            
            // ---- xem nếu có gần hơn vật thể trước
            if( cachXa < kVO_CUC ) {
//               printf( "vatTheloai %d  cachXa %5.3f  *soLuongVatTheToMauBong %d\n", vatThe->loai, cachXa, *soLuongVatTheToMauBong );
               thongTinToMauBong[*soLuongVatTheToMauBong].vatThe = vatThe;
               thongTinToMauBong[*soLuongVatTheToMauBong].diemTrungTDVT = diemTrungTDVT;
               thongTinToMauBong[*soLuongVatTheToMauBong].phapTuyenTDVT = phapTuyenTDVT;
//               cachXaGanNhat = cachXa;
//               printf( "xemCoTrungVat: thongtin.phapTueyn %5.3f %5.3f %5.3f\n", thongTinToMauBong->phapTuyenTDVT.x, thongTinToMauBong->phapTuyenTDVT.y, thongTinToMauBong->phapTuyenTDVT.z );
//               printf( "xemCoTrungVat: phapTueyn %5.3f %5.3f %5.3f\n", phapTuyenTDVT.x, phapTuyenTDVT.y, phapTuyenTDVT.z );
               thongTinToMauBong[*soLuongVatTheToMauBong].cachXa = cachXa;
               (*soLuongVatTheToMauBong)++;
//               return kDUNG;  // kiếm vật thể cần nhất
            }
            
            if( *soLuongVatTheToMauBong == kSO_LUONG__VAT_THE_BONG_TOI_DA ) {
               return kDUNG;
            }
         }
      }
   }
   else { // số lượng giao điểm == 0, không có vật thể
//      printf( "xemTiaCóTrúngVậtThểNào: SAI LẦM số lượng giao điểm == 0\n" );
      // ---- không có vật thể, không thể trúng vật thể nào
      thongTinToMauBong[*soLuongVatTheToMauBong].vatThe = NULL;
      thongTinToMauBong[*soLuongVatTheToMauBong].cachXa = kVO_CUC;
      return kSAI;
   }
   
   // ---- không trúng vật thể nào
   if( thongTinToMauBong->vatThe == NULL ) {
      thongTinToMauBong->cachXa = kVO_CUC;
      return kSAI;
   }
   // ---- có trúng vật thể
   else {
      return kDUNG;
   }
}


unsigned char xemTiaCatMatPhangVaTruc( Tia *tia, float toaDoMatPhang, unsigned char truc, unsigned char ben ) {
   
   unsigned char cat = kDUNG;
   
   if( truc == kTRUC_X ) {
      if( ben == kBEN_TRAI ) {  // nếu thành phần hướng x = 0.0f, tia songๆ với mặt phẳng và không thể cặt nó
         if( (tia->huong.x >= 0.0f) && (tia->goc.x > toaDoMatPhang) )
            cat = kSAI;
      }
      else { // if( ben == kBEN_PHAI ) {
         if( (tia->huong.x <= 0.0f) && (tia->goc.x < toaDoMatPhang) )
            cat = kSAI;
      }
   }
   else if( truc == kTRUC_Y ) {
      if( ben == kBEN_TRAI ) {  // nếu thành phần hướng x = 0.0f, tia songๆ với mặt phẳng và không thể cặt nó
         if( (tia->huong.y >= 0.0f) && (tia->goc.y > toaDoMatPhang) )
            cat = kSAI;
      }
      else { // if( ben == kBEN_PHAI ) {
         if( (tia->huong.y <= 0.0f) && (tia->goc.y < toaDoMatPhang) )
            cat = kSAI;
      }
   }
   else {  // if( truc == kTRUC_Z ) {
      if( ben == kBEN_TRAI ) {  // nếu thành phần hướng x = 0.0f, tia songๆ với mặt phẳng và không thể cặt nó
         if( (tia->huong.z >= 0.0f) && (tia->goc.z > toaDoMatPhang) )
            cat = kSAI;
      }
      else { // if( ben == kBEN_PHAI ) {
         if( (tia->huong.z <= 0.0f) && (tia->goc.z < toaDoMatPhang) )
            cat = kSAI;
      }
   }
   //   printf( "truc %d  ben %d  toaDo %5.3f  cat %d   tia  %5.3f %5.3f %5.3f  %5.3f %5.3f %5.3f\n", truc, ben, toaDoMatPhang, cat, tia->goc.x, tia->goc.y, tia->goc.z, tia->huong.x, tia->huong.y, tia->huong.z );
   return cat;
}


float xemTiaCoTrungVatTrongKhongGianVat( VatThe *vatThe, Tia *tia, Vecto *phapTuyen, Vecto *diemTrungTDVT ) {

 //  printf( "xemTiaCat: tia %5.3f %5.3f %5.3f   %5.3f %5.3f %5.3f\n", tia->goc.x, tia->goc.y, tia->goc.z, tia->huong.x, tia->huong.y, tia->huong.z );

   float cachXa = kVO_CUC;
   unsigned char cat = xemCatBaoBi( &(vatThe->baoBiVT), tia );

   if( cat ) {

      if( vatThe->loai == kLOAI_HINH_DANG__HINH_CAU ) {
         HinhCau *hinhCau = &(vatThe->hinhDang.hinhCau);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatHinhCau( hinhCau, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__MAT_PHANG ) {
         MatPhang *matPhang = &(vatThe->hinhDang.matPhang);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatMatPhang( matPhang, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__HOP ) {
         Hop *hop = &(vatThe->hinhDang.hop);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatHop( hop, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__HINH_TRU ) {
         HinhTru *hinhTru = &(vatThe->hinhDang.hinhTru );
         cachXa = xemCatHinhTru( hinhTru, tia, phapTuyen, diemTrungTDVT );
         //      printf( "xemCat: cachXa %5.3f\n", cachXa );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__HINH_NON ) {
         HinhNon *hinhNon = &(vatThe->hinhDang.hinhNon);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatHinhNon( hinhNon, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__MAT_HYPERBOL ) {
         MatHyperbol *matHyperbol = &(vatThe->hinhDang.matHyperbol);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatMatHyperbol( matHyperbol, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__MAT_PARABOL ) {
         MatParabol *matParabol = &(vatThe->hinhDang.matParabol);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatMatParabol( matParabol, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__MAT_SONG ) {
         MatSong *matSong = &(vatThe->hinhDang.matSong);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatMatSong( matSong, tia, phapTuyen, diemTrungTDVT, (float)thoiGian );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__HINH_XUYEN ) {
         HinhXuyen *hinhXuyen = &(vatThe->hinhDang.hinhXuyen);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatHinhXuyen( hinhXuyen, tia, phapTuyen, diemTrungTDVT, (float)thoiGian );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__TU_DIEN ) {
         TuDien *tuDien = &(vatThe->hinhDang.tuDien);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatVatTheTamGiac( tuDien->mangDinh, tuDien->mangTamGiac, tuDien->soLuongTamGiac,
                                      tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__BAT_DIEN ) {
         BatDien *batDien = &(vatThe->hinhDang.batDien);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatVatTheTamGiac( batDien->mangDinh, batDien->mangTamGiac, batDien->soLuongTamGiac, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__THAP_NHI_DIEN ) {
         ThapNhiDien *thapNhiDien = &(vatThe->hinhDang.thapNhiDien);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatVatTheTamGiac( thapNhiDien->mangDinh, thapNhiDien->mangTamGiac, thapNhiDien->soLuongTamGiac, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__KIM_TU_THAP ) {
         KimTuThap *kimTuThap = &(vatThe->hinhDang.kimTuThap);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatVatTheTamGiac( kimTuThap->mangDinh, kimTuThap->mangTamGiac, kimTuThap->soLuongTamGiac, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__DOC ) {
         Doc *doc = &(vatThe->hinhDang.doc);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatVatTheTamGiac( doc->mangDinh, doc->mangTamGiac, doc->soLuongTamGiac, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__NHI_THAP_DIEN ) {
         NhiThapDien *nhiThapDien = &(vatThe->hinhDang.nhiThapDien);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatVatTheTamGiac( nhiThapDien->mangDinh, nhiThapDien->mangTamGiac, nhiThapDien->soLuongTamGiac, tia, phapTuyen, diemTrungTDVT );
      }
      else if( vatThe->loai == kLOAI_HINH_DANG__SAO_GAI ) {
         SaoGai *saoGai = &(vatThe->hinhDang.saoGai);
         // ---- kiếm cách xa (cách xa theo đơn vị hướng tia, không phải đơn vị thế giới)
         cachXa = xemCatVatTheTamGiac( saoGai->mangDinh, saoGai->mangTamGiac, saoGai->soLuongTamGiac, tia, phapTuyen, diemTrungTDVT );
      }
      // -----
      else if( vatThe->loai == kLOAI_VAT_THE__BOOL ) {
         cachXa = xemTiaCoTrungVatBoolTrongKhongGianVat( vatThe->danhSachVatThe, vatThe->soLuongVatThe, vatThe->mucDichBool, tia, phapTuyen, diemTrungTDVT );
         //      printf( "cachXa %5.3f   diem %5.3f %5.3f %5.3f   phap %5.3f %5.3f %5.3f\n", cachXa, diemTrungTDVT->x, diemTrungTDVT->y, diemTrungTDVT->z, phapTuyen->x, phapTuyen->y, phapTuyen->z );
      }
      else if( vatThe->loai == kLOAI_VAT_THE__GHEP ) {
         cachXa = xemTiaCoTrungVatGhepTrongKhongGianVat( vatThe->danhSachVatThe, vatThe->soLuongVatThe, tia, phapTuyen, diemTrungTDVT );
      }
   }

   // ---- biến đổi pháp tuyến trở lại tọa độ thấ giới
   if( cachXa < kVO_CUC ) {
      Vecto phapTuyenTDTG;
      phapTuyenTDTG = nhanVectoVoiMaTran3x3( phapTuyen, vatThe->nghichBienHoaChoPhapTuyen );
      phapTuyen->x = phapTuyenTDTG.x;
      phapTuyen->y = phapTuyenTDTG.y;
      phapTuyen->z = phapTuyenTDTG.z;
   }

   return cachXa;
}


// -----
// Cần đoạn xuyên qua vật: Điểm vào và Điểm ra
// - cần biết điểm ra hay vào gì có thể phát tia ở trong vật thể và chỉ đi ra ngoài (không có vào)
// - cần pháp tuyến toạ độ thế giới và điểm trúng tọa độ vật thể (cho tô màu) 
// giá trị vật thể +1 hay -1

typedef struct {
   float cach;
   Vecto diemTrungTDVT;
   Vecto phapTuyen;
   char giaTri;     // giá trị bool tùy vào hay ra vật và giá trị vật thể
   char giaTriBool; // giá trị bool của vật thể đang xuyên qua
   unsigned char chiSoVatBool;  // chỉ số vật thể trong danh sách bool
} Doan;

#define kSO_LUONG__VAT_THE_BOOL 32

float xemTiaCoTrungVatBoolTrongKhongGianVat( VatThe *danhSachVatTheBool, unsigned char soLuongVatThe, unsigned char mucDichBool, Tia *tia, Vecto *phapTuyen, Vecto *diemTrungTDVT ) {

   if( soLuongVatThe > kSO_LUONG__VAT_THE_BOOL )
      soLuongVatThe = kSO_LUONG__VAT_THE_BOOL;

   Doan doan[kSO_LUONG__VAT_THE_BOOL << 3];
   char giaTriTrongVatThe[kSO_LUONG__VAT_THE_BOOL << 2];

   // ---- trước tìm hết đoạn ở trong vật thể (nếu có)
   unsigned char soDoan = 0;
   unsigned char soVat = 0;
   while( soVat < soLuongVatThe ) {
      VatThe *vatThe = &(danhSachVatTheBool[soVat]);
      Tia tiaBienHoa0;
      // ---- biến hóa tia đến không gian vật thể  [gốc]•[dịch]-1 •[xoay]-1 •[phóng to]-1
      tiaBienHoa0.goc = nhanVectoVoiMaTran4x4( &(tia->goc), vatThe->nghichBienHoa );
      // ---- đừng dịch hướng, chỉ [hướng]•[xoay]-1 •[phóng to]-1
      tiaBienHoa0.huong = nhanVectoVoiMaTran3x3( &(tia->huong), vatThe->nghichBienHoa );
      Vecto diemTrungVatThe;
      Vecto phapTuyenVatThe;
      float cachXa0 = xemTiaCoTrungVatTrongKhongGianVat( vatThe, &tiaBienHoa0, &phapTuyenVatThe, &diemTrungVatThe );
      
      // ---- xem nếu góc tia trong vật thể
      if( xemDiemTrongVatThe( vatThe, &(tiaBienHoa0.goc) ) )
         giaTriTrongVatThe[soVat] = vatThe->giaTri;
      else
         giaTriTrongVatThe[soVat] = 0;

      // ---- xem nếu chúng cái gì
      if( cachXa0 < kVO_CUC ) {
         // ---- xem dấu tích vô hướng tia với pháp tuyến (cho biết vào hay ra)
         float tichVoHuong = phapTuyenVatThe.x*tia->huong.x + phapTuyenVatThe.y*tia->huong.y + phapTuyenVatThe.z*tia->huong.z;
         doan[soDoan].giaTriBool = vatThe->giaTri;
         if( vatThe->giaTri == 1 ) {
            if( tichVoHuong < 0.0f )  // vào vật thể +
               doan[soDoan].giaTri = +1;
            else       // ra vật thể +
               doan[soDoan].giaTri = -1;
         }
         else if( vatThe->giaTri == -1 ) {
            if( tichVoHuong < 0.0f )  // vào vật thể -
               doan[soDoan].giaTri = -1;
            else       // ra vật thể -
               doan[soDoan].giaTri = +1;
         }
         // ---- giữ thông tin cho đoạn
         doan[soDoan].cach = cachXa0;
         // ---- tính pháp tuyến
         Vecto phapTuyen;
         phapTuyen = nhanVectoVoiMaTran3x3( &phapTuyenVatThe, vatThe->nghichBienHoaChoPhapTuyen );
         doan[soDoan].phapTuyen.x = phapTuyen.x;
         doan[soDoan].phapTuyen.y = phapTuyen.y;
         doan[soDoan].phapTuyen.z = phapTuyen.z;
         // ---- điểm trúng
         doan[soDoan].diemTrungTDVT.x = diemTrungVatThe.x;
         doan[soDoan].diemTrungTDVT.y = diemTrungVatThe.y;
         doan[soDoan].diemTrungTDVT.z = diemTrungVatThe.z;
         
         doan[soDoan].chiSoVatBool = soVat;
         soDoan++;
   
         // ---- phát tia tiếp đến bên kia nếu có
         if( tichVoHuong < 0.0f ) {  // đang vào
             // ---- tính tia tiếp từ điểm trúng (cộng thêm 0.1f cho không trúng điểm cũ)
            Tia tiaBienHoa1;
            tiaBienHoa1.goc.x = tiaBienHoa0.goc.x + cachXa0*tiaBienHoa0.huong.x;
            tiaBienHoa1.goc.y = tiaBienHoa0.goc.y + cachXa0*tiaBienHoa0.huong.y;
            tiaBienHoa1.goc.z = tiaBienHoa0.goc.z + cachXa0*tiaBienHoa0.huong.z;
            tiaBienHoa1.goc.x += 0.005f*tiaBienHoa0.huong.x;
            tiaBienHoa1.goc.y += 0.005f*tiaBienHoa0.huong.y;
            tiaBienHoa1.goc.z += 0.005f*tiaBienHoa0.huong.z;
            tiaBienHoa1.huong = tiaBienHoa0.huong;
            float cachXa1 = xemTiaCoTrungVatTrongKhongGianVat( vatThe, &tiaBienHoa1, &phapTuyenVatThe, &diemTrungVatThe );
            // ---- tính tích vô hướng
            float tichVoHuong = phapTuyenVatThe.x*tia->huong.x + phapTuyenVatThe.y*tia->huong.y + phapTuyenVatThe.z*tia->huong.z;

            if( cachXa1 < kVO_CUC ) {
               doan[soDoan].giaTriBool = vatThe->giaTri;
               if( vatThe->giaTri == 1 ) {
                  if( tichVoHuong < 0.0f )  // vào vật thể +
                     printf( "XemCắtBool: SAI LẦM Không nên vào vật %d cachXa0 %5.3f  cachXa1 %5.3f\n", soVat, cachXa0, cachXa1 );
                  else       // ra vật thể +
                     doan[soDoan].giaTri = -1;
               }
               else if( vatThe->giaTri == -1 ) {
                  if( tichVoHuong < 0.0f )  // vào vật thể -
                     printf( "XemCắtBool: SAI LẦM Không nên vào vật %d  cachXa0 %5.3f  cachXa1 %5.3f\n", soVat, cachXa0, cachXa1 );
                  else       // ra vật thể -
                     doan[soDoan].giaTri = +1;
               }
               // ---- giữ thông tin cho đoạn
               doan[soDoan].cach = cachXa1 + cachXa0;
               // ---- tính pháp tuyến
               phapTuyen = nhanVectoVoiMaTran3x3( &phapTuyenVatThe, vatThe->nghichBienHoaChoPhapTuyen );
               doan[soDoan].phapTuyen.x = phapTuyen.x;
               doan[soDoan].phapTuyen.y = phapTuyen.y;
               doan[soDoan].phapTuyen.z = phapTuyen.z;
               // ---- giữ điểm trúng
               doan[soDoan].diemTrungTDVT.x = diemTrungVatThe.x;
               doan[soDoan].diemTrungTDVT.y = diemTrungVatThe.y;
               doan[soDoan].diemTrungTDVT.z = diemTrungVatThe.z;
               
               doan[soDoan].chiSoVatBool = soVat;
               soDoan++;
            }
            else {  // tia đã cắt một góc và không thất mặt kia, bỏ đoàn này
               // thật nên xem nếu ờ trong vật thể trước bỏ đoàn này
               soDoan--;
            }
         }
      }
      soVat++;
   }
   
   // ----
   
   // ---- xử lý: cần kiếm điểm gần nhất mà giá trị đém = 1
   if( soDoan > 0 ) {
      // ---- chuần bị mảng chỉ số
      unsigned char soLuongDoan = soDoan;
      soDoan = 0;
      unsigned char mangChiSo[16];
      while ( soDoan < soLuongDoan ) {
         mangChiSo[soDoan] = soDoan;
         soDoan++;
      }

      // ---- sắp xếp lại từ cách gần nhất đến xa nhất
      unsigned char soVong = 0;
      while( soVong < soLuongDoan - 1 ) {
         soDoan = 0;
         while ( soDoan < soLuongDoan-1 ) {
            if( doan[mangChiSo[soDoan]].cach > doan[mangChiSo[soDoan+1]].cach ) {
               unsigned char chiSo = mangChiSo[soDoan];
               mangChiSo[soDoan] = mangChiSo[soDoan+1];
               mangChiSo[soDoan+1]= chiSo;
            }
            soDoan++;
         }
         soVong++;
      }
      
      // ---- kiếm cách gần nhất
      char dem = 0;
      // ---- đặt giá trị đếm bằng tổng giả trị của vật thể góc tia đang ở trong
      // ---- Xem nếu điểm này ở trong vật thể nào nếu không ở trong vật thể nào, đếm sẽ = 0
      unsigned char soVat = 0;
      while ( soVat < soLuongVatThe ) {
         dem += giaTriTrongVatThe[soVat];
         soVat++;
      }
   
      // ---- tìm điểm trúng có giá trị bool đến mục đích
      soDoan = 0;
      while ( (soDoan < soLuongDoan) && (dem < mucDichBool) ) {
         dem += doan[mangChiSo[soDoan]].giaTri;
//         printf( "mangChiSo[%d] = %d  %5.3f  %d  %d\n", soDoan, mangChiSo[soDoan], doan[mangChiSo[soDoan]].cach, dem, doan[mangChiSo[soDoan]].giaTriBool );
         soDoan++;
      }
      
//      printf( "dem %d\n\n", dem );
      if( (dem > 0) && soDoan ) {


         soDoan--;
         diemTrungTDVT->x = doan[mangChiSo[soDoan]].diemTrungTDVT.x;
         diemTrungTDVT->y = doan[mangChiSo[soDoan]].diemTrungTDVT.y;
         diemTrungTDVT->z = doan[mangChiSo[soDoan]].diemTrungTDVT.z;

         // ---- vật âm, lật pháp tuyến;
         Vecto phapTuyenTDVT; // toạ độ vật thể thành phần bool
         if( doan[mangChiSo[soDoan]].giaTriBool < 0 ) {
            phapTuyenTDVT.x = -doan[mangChiSo[soDoan]].phapTuyen.x;
            phapTuyenTDVT.y = -doan[mangChiSo[soDoan]].phapTuyen.y;
            phapTuyenTDVT.z = -doan[mangChiSo[soDoan]].phapTuyen.z;
         }
         else {
            // ---- lật hướng
            phapTuyenTDVT.x = doan[mangChiSo[soDoan]].phapTuyen.x;
            phapTuyenTDVT.y = doan[mangChiSo[soDoan]].phapTuyen.y;
            phapTuyenTDVT.z = doan[mangChiSo[soDoan]].phapTuyen.z;
         }
         // ---- biến hóa pháp tuyến sang toạ độ vật thể phụ huynh
         unsigned char chiSoVatBool = doan[mangChiSo[soDoan]].chiSoVatBool;
         
         Vecto phapTuyenTDTG = nhanVectoVoiMaTran3x3( &phapTuyenTDVT, danhSachVatTheBool[doan[mangChiSo[soDoan]].chiSoVatBool].nghichBienHoa );
         phapTuyen->x = phapTuyenTDTG.x;
         phapTuyen->y = phapTuyenTDTG.y;
         phapTuyen->z = phapTuyenTDTG.z;

         return doan[mangChiSo[soDoan]].cach;
      }
      else {
         return kVO_CUC;
      }
   }
   else {
         return kVO_CUC;
   }
}

float xemTiaCoTrungVatGhepTrongKhongGianVat( VatThe *danhSachVatTheBool, unsigned char soLuongVatThe, Tia *tia, Vecto *phapTuyen, Vecto *diemTrungTDVT ) {
   
   float cachXaGanNhat = kVO_CUC;
   Vecto phapTuyenGanNhat;
   Vecto diemTrungGanNhatTDVT;
   
   unsigned char soVat = 0;
   while( soVat < soLuongVatThe ) {
      VatThe *vatThe = &(danhSachVatTheBool[soVat]);
      Tia tiaBienHoa0;
      // ---- biến hóa tia đến không gian vật thể  [gốc]•[dịch]-1 •[xoay]-1 •[phóng to]-1
      tiaBienHoa0.goc = nhanVectoVoiMaTran4x4( &(tia->goc), vatThe->nghichBienHoa );
      // ---- đừng dịch hướng, chỉ [hướng]•[xoay]-1 •[phóng to]-1
      tiaBienHoa0.huong = nhanVectoVoiMaTran3x3( &(tia->huong), vatThe->nghichBienHoa );
      Vecto diemTrungVatThe;
      Vecto phapTuyenVatThe;
      float cachXa = xemTiaCoTrungVatTrongKhongGianVat( vatThe, &tiaBienHoa0, &phapTuyenVatThe, &diemTrungVatThe );
      // ---- xem nếu có gần hơn vật thể trước
      if( cachXa < cachXaGanNhat ) {
         cachXaGanNhat = cachXa;
         phapTuyen->x = phapTuyenVatThe.x;
         phapTuyen->y = phapTuyenVatThe.y;
         phapTuyen->z = phapTuyenVatThe.z;
         diemTrungTDVT->x = diemTrungVatThe.x;
         diemTrungTDVT->y = diemTrungVatThe.y;
         diemTrungTDVT->z = diemTrungVatThe.z;
      }
      soVat++;
   }
   
   return cachXaGanNhat;
}

unsigned char xemDiemTrongVatThe( VatThe *vatThe, Vecto *diem ) {
   
   unsigned char diemTrong = kSAI;  // đặt sẵn không có trong vật thể
   if( vatThe->loai == kLOAI_HINH_DANG__HINH_CAU )
      diemTrong = xemDiemTrongHinhCau( &(vatThe->hinhDang.hinhCau), diem );

   else if( vatThe->loai == kLOAI_HINH_DANG__HOP )
      diemTrong = xemDiemTrongHop( &(vatThe->hinhDang.hop), diem );
   
   else if( vatThe->loai == kLOAI_HINH_DANG__HINH_TRU )
      diemTrong = xemDiemTrongHinhTru( &(vatThe->hinhDang.hinhTru), diem );
   
   else if( vatThe->loai == kLOAI_HINH_DANG__HINH_NON )
      diemTrong = xemDiemTrongHinhNon( &(vatThe->hinhDang.hinhNon), diem );
   
   return diemTrong;
}

#pragma mark ---- Bao Bì
// hop 6 float [-x, +x, -y, +y, -z, +z]  dấu == 1 nếu âm
// tia.goc.x + t*tia.huong.x = x
// t = (x - tia.goc.x)/tia.huong
unsigned char xemCatBaoBi( BaoBi *baoBiVT, Tia *tia ) {
   
//   printf( "baoBi: hop %5.3f %5.3f %5.3f %5.3f %5.3f %5.3f\n", baoBiVT->gocCucTieu.x, baoBiVT->gocCucDai.x, baoBiVT->gocCucTieu.y, baoBiVT->gocCucDai.y, baoBiVT->gocCucTieu.z, baoBiVT->gocCucDai.z );
//   printf( "baoBi: tia %5.3f %5.3f %5.3f   %5.3f %5.3f %5.3f\n", tia->goc.x, tia->goc.y, tia->goc.z, tia->huong.x, tia->huong.y, tia->huong.z );
   unsigned char trung = kSAI;

   // ---- tính nghiệm cho hướng x
   if( tia->huong.x != 0.0f ) {
//      printf( "baoBi: x\n");
      // ---- tính hai nghiệm
      float nghiem0 = (baoBiVT->gocCucTieu.x - tia->goc.x) / tia->huong.x;
      float nghiem1 = (baoBiVT->gocCucDai.x - tia->goc.x) / tia->huong.x;

      // ----
      if( nghiem0 < 0.0001f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.0001f )
         nghiem1 = kVO_CUC;
      
      if( nghiem0 < kVO_CUC ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungX;
         //         diemTrungX.x = tia->goc.x + nghiem0*tia->huong.x;
         diemTrungX.y = tia->goc.y + nghiem0*tia->huong.y;
         diemTrungX.z = tia->goc.z + nghiem0*tia->huong.z;

         if( diemTrungX.y > baoBiVT->gocCucDai.y )
            trung = kSAI;
         else if( diemTrungX.y < baoBiVT->gocCucTieu.y )
            trung = kSAI;
         else if( diemTrungX.z > baoBiVT->gocCucDai.z )
            trung = kSAI;
         else if( diemTrungX.z < baoBiVT->gocCucTieu.z )
            trung = kSAI;
         else
            trung = kDUNG;
      }
      if( nghiem1 < kVO_CUC && trung == kSAI ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungX;
         //         diemTrungX.x = tia->goc.x + nghiem1*tia->huong.x;
         diemTrungX.y = tia->goc.y + nghiem1*tia->huong.y;
         diemTrungX.z = tia->goc.z + nghiem1*tia->huong.z;
         //      printf("nghiemX %5.3f  %5.3f\n", nghiemX, nghiemGanNhat );
         if( diemTrungX.y > baoBiVT->gocCucDai.y )
            trung = kSAI;
         else if( diemTrungX.y < baoBiVT->gocCucTieu.y )
            trung = kSAI;
         else if( diemTrungX.z > baoBiVT->gocCucDai.z )
            trung = kSAI;
         else if( diemTrungX.z < baoBiVT->gocCucTieu.z )
            trung = kSAI;
         else
            trung = kDUNG;
      }
   }
   
   // ---- tính nghiệm cho hướng y
   if( tia->huong.y != 0.0f && trung == kSAI ) {
//      printf( "baoBi: y\n ");
      // ---- tính hai nghiệm
      float nghiem0 = (baoBiVT->gocCucTieu.y - tia->goc.y) / tia->huong.y;
      float nghiem1 = (baoBiVT->gocCucDai.y - tia->goc.y) / tia->huong.y;
      
      // ----
      if( nghiem0 < 0.0001f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.0001f )
         nghiem1 = kVO_CUC;
      
      if( nghiem0 < kVO_CUC ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungY;
         diemTrungY.x = tia->goc.x + nghiem0*tia->huong.x;
         //         diemTrungY.y = tia->goc.y + nghiem0*tia->huong.y;
         diemTrungY.z = tia->goc.z + nghiem0*tia->huong.z;
         //      printf("nghiemX %5.3f  %5.3f\n", nghiemX, nghiemGanNhat );
         if( diemTrungY.x > baoBiVT->gocCucDai.x )
            trung = kSAI;
         else if( diemTrungY.x < baoBiVT->gocCucTieu.x )
            trung = kSAI;
         else if( diemTrungY.z > baoBiVT->gocCucDai.z )
            trung = kSAI;
         else if( diemTrungY.z < baoBiVT->gocCucTieu.z )
            trung = kSAI;
         else
            trung = kDUNG;
      }
      if( nghiem1 < kVO_CUC && trung == kSAI ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungY;
         diemTrungY.x = tia->goc.x + nghiem1*tia->huong.x;
         //         diemTrungY.y = tia->goc.y + nghiem1*tia->huong.y;
         diemTrungY.z = tia->goc.z + nghiem1*tia->huong.z;
         //      printf("nghiemX %5.3f  %5.3f\n", nghiemX, nghiemGanNhat );
         if( diemTrungY.x > baoBiVT->gocCucDai.x )
            trung = kSAI;
         else if( diemTrungY.x < baoBiVT->gocCucTieu.x )
            trung = kSAI;
         else if( diemTrungY.z > baoBiVT->gocCucDai.z )
            trung = kSAI;
         else if( diemTrungY.z < baoBiVT->gocCucTieu.z )
            trung = kSAI;
         else
            trung = kDUNG;
      }
   }
//   printf( "tia->huong.z %5.3f  trung %d\n", tia->huong.z, trung );
   // ---- tính nghiệm cho hướng z
   if( tia->huong.z != 0.0f && trung == kSAI ) {

      // ---- tính hai nghiệm
      float nghiem0 = (baoBiVT->gocCucTieu.z - tia->goc.z) / tia->huong.z;
      float nghiem1 = (baoBiVT->gocCucDai.z - tia->goc.z) / tia->huong.z;
//      printf( "baoBi: z nghiem0 %5.3f    nghiem1 %5.3e\n", nghiem0, nghiem1 );
      // ----
      if( nghiem0 < 0.0001f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.0001f )
         nghiem1 = kVO_CUC;
//      printf( "baoBi: z 2 nghiem0 %5.3f    nghiem1 %5.3e\n", nghiem0, nghiem1 );
      
      if( nghiem0 < kVO_CUC ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungZ;
         diemTrungZ.x = tia->goc.x + nghiem0*tia->huong.x;
         diemTrungZ.y = tia->goc.y + nghiem0*tia->huong.y;
         //         diemTrungZ.z = tia->goc.z + nghiem0*tia->huong.z;
//         printf("diemTrungZ %5.3f  %5.3f\n", diemTrungZ.x, diemTrungZ.y );
         if( diemTrungZ.x > baoBiVT->gocCucDai.x )
            trung = kSAI;
         else if( diemTrungZ.x < baoBiVT->gocCucTieu.x )
            trung = kSAI;
         else if( diemTrungZ.y > baoBiVT->gocCucDai.y )
            trung = kSAI;
         else if( diemTrungZ.y < baoBiVT->gocCucTieu.y )
            trung = kSAI;
         else
            trung = kDUNG;
      }
      
//      printf("trung %d\n", trung );
      
      if( nghiem1 < kVO_CUC && trung == kSAI ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungZ;
         diemTrungZ.x = tia->goc.x + nghiem1*tia->huong.x;
         diemTrungZ.y = tia->goc.y + nghiem1*tia->huong.y;
         //         diemTrungZ.z = tia->goc.z + nghiem1*tia->huong.z;
         //      printf("nghiemX %5.3f  %5.3f\n", nghiemX, nghiemGanNhat );
         if( diemTrungZ.x > baoBiVT->gocCucDai.x )
            trung = kSAI;
         else if( diemTrungZ.x < baoBiVT->gocCucTieu.x )
            trung = kSAI;
         else if( diemTrungZ.y > baoBiVT->gocCucDai.y )
            trung = kSAI;
         else if( diemTrungZ.y < baoBiVT->gocCucTieu.y )
            trung = kSAI;
         else
            trung = kDUNG;
      }
   }
//   printf("baoBi: trung %d\n\n", trung );

   return trung; // ( (tmin < t1) && (tmax > t0) );
}

BaoBi tinhBaoBiTGChoDanhSachVatThe( VatThe *danhSachVatThe, unsigned short soLuongVatThe ) {

   BaoBi baoBiPhimTruong;
   if( soLuongVatThe == 0 )
      printf( "TínhBaoBìChoPhimTrường: Số lượng vật thể = 0\n" );
   else {
      // ---- đặt bao bì phim trường bằng bao bì vật thể đầu
      tinhBaoBiTGChoVatThe( &(danhSachVatThe[0]) );
      baoBiPhimTruong = danhSachVatThe[0].baoBiTG;

      // ----
      unsigned short chiSo = 1;
      while ( chiSo < soLuongVatThe ) {
//         printf( "soVatThe %d\n", chiSo );
         tinhBaoBiTGChoVatThe( &(danhSachVatThe[chiSo]) );
   
         BaoBi baoBiTG = danhSachVatThe[chiSo].baoBiTG;
         // ---- X
         if( baoBiTG.gocCucTieu.x < baoBiPhimTruong.gocCucTieu.x )
            baoBiPhimTruong.gocCucTieu.x = baoBiTG.gocCucTieu.x;
         if( baoBiTG.gocCucDai.x > baoBiPhimTruong.gocCucDai.x )
            baoBiPhimTruong.gocCucDai.x = baoBiTG.gocCucDai.x;
         // ---- Y
         if( baoBiTG.gocCucTieu.y < baoBiPhimTruong.gocCucTieu.y )
            baoBiPhimTruong.gocCucTieu.y = baoBiTG.gocCucTieu.y;
         if( baoBiTG.gocCucDai.y > baoBiPhimTruong.gocCucDai.y )
            baoBiPhimTruong.gocCucDai.y = baoBiTG.gocCucDai.y;
         // ---- Z
         if( baoBiTG.gocCucTieu.z < baoBiPhimTruong.gocCucTieu.z )
            baoBiPhimTruong.gocCucTieu.z = baoBiTG.gocCucTieu.z;
         if( baoBiTG.gocCucDai.z > baoBiPhimTruong.gocCucDai.z )
            baoBiPhimTruong.gocCucDai.z = baoBiTG.gocCucDai.z;
         chiSo++;
      }
   }

   return baoBiPhimTruong;
}

void tinhBaoBiTGChoVatThe( VatThe *vatThe ) {
   
   // ---- xem loại vật thể, tính bao bì cho vật thể ghép khác cho vật thể ghép và bool
   unsigned char loaiVatThe = vatThe->loai;
   // ---- biến hóa thành tọa độ thế giới
 //  printf( "loaiVatThe %d\n", loaiVatThe );
    {
      // ---- tính các vectơ sẽ cần (một vectơ đến mỗi góc -> 8 vectơ)
      Vecto goc_xAm_yAm_zAm_TG;
      Vecto goc_xDuong_yAm_zAm_TG;
      Vecto goc_xAm_yAm_zDuong_TG;
      Vecto goc_xDuong_yAm_zDuong_TG;
      
      Vecto goc_xAm_yDuong_zAm_TG;
      Vecto goc_xDuong_yDuong_zAm_TG;
      Vecto goc_xAm_yDuong_zDuong_TG;
      Vecto goc_xDuong_yDuong_zDuong_TG;
      
      // ---- vectơ 8 góc tương đối với vật thể
      Vecto goc_xAm_yAm_zAm_VT;
      goc_xAm_yAm_zAm_VT.x = vatThe->baoBiVT.gocCucTieu.x;
      goc_xAm_yAm_zAm_VT.y = vatThe->baoBiVT.gocCucTieu.y;
      goc_xAm_yAm_zAm_VT.z = vatThe->baoBiVT.gocCucTieu.z;
      
      Vecto goc_xDuong_yAm_zAm_VT;
      goc_xDuong_yAm_zAm_VT.x = vatThe->baoBiVT.gocCucDai.x;
      goc_xDuong_yAm_zAm_VT.y = vatThe->baoBiVT.gocCucTieu.y;
      goc_xDuong_yAm_zAm_VT.z = vatThe->baoBiVT.gocCucTieu.z;
      
      Vecto goc_xAm_yAm_zDuong_VT;
      goc_xAm_yAm_zDuong_VT.x = vatThe->baoBiVT.gocCucTieu.x;
      goc_xAm_yAm_zDuong_VT.y = vatThe->baoBiVT.gocCucTieu.y;
      goc_xAm_yAm_zDuong_VT.z = vatThe->baoBiVT.gocCucDai.z;
      
      Vecto goc_xDuong_yAm_zDuong_VT;
      goc_xDuong_yAm_zDuong_VT.x = vatThe->baoBiVT.gocCucDai.x;
      goc_xDuong_yAm_zDuong_VT.y = vatThe->baoBiVT.gocCucTieu.y;
      goc_xDuong_yAm_zDuong_VT.z = vatThe->baoBiVT.gocCucDai.z;
      // ----
      Vecto goc_xAm_yDuong_zAm_VT;
      goc_xAm_yDuong_zAm_VT.x = vatThe->baoBiVT.gocCucTieu.x;
      goc_xAm_yDuong_zAm_VT.y = vatThe->baoBiVT.gocCucDai.y;
      goc_xAm_yDuong_zAm_VT.z = vatThe->baoBiVT.gocCucTieu.z;
      
      Vecto goc_xDuong_yDuong_zAm_VT;
      goc_xDuong_yDuong_zAm_VT.x = vatThe->baoBiVT.gocCucDai.x;
      goc_xDuong_yDuong_zAm_VT.y = vatThe->baoBiVT.gocCucDai.y;
      goc_xDuong_yDuong_zAm_VT.z = vatThe->baoBiVT.gocCucTieu.z;
      
      Vecto goc_xAm_yDuong_zDuong_VT;
      goc_xAm_yDuong_zDuong_VT.x = vatThe->baoBiVT.gocCucTieu.x;
      goc_xAm_yDuong_zDuong_VT.y = vatThe->baoBiVT.gocCucDai.y;
      goc_xAm_yDuong_zDuong_VT.z = vatThe->baoBiVT.gocCucDai.z;
      
      Vecto goc_xDuong_yDuong_zDuong_VT;
      goc_xDuong_yDuong_zDuong_VT.x = vatThe->baoBiVT.gocCucDai.x;
      goc_xDuong_yDuong_zDuong_VT.y = vatThe->baoBiVT.gocCucDai.y;
      goc_xDuong_yDuong_zDuong_VT.z = vatThe->baoBiVT.gocCucDai.z;
      
     /*
      printf( "%5.3f %5.3f\n", goc_xAm_yAm_zAm_VT.x, goc_xAm_yAm_zAm_VT.y, goc_xAm_yAm_zAm_VT.z );
      printf( "%5.3f %5.3f\n", goc_xDuong_yAm_zAm_VT.x, goc_xDuong_yAm_zAm_VT.y, goc_xAm_yDuong_zAm_VT.z );
      printf( "%5.3f %5.3f\n", goc_xAm_yAm_zDuong_VT.x, goc_xAm_yAm_zDuong_VT.y, goc_xAm_yAm_zDuong_VT.z );
      printf( "%5.3f %5.3f\n\n", goc_xDuong_yAm_zDuong_VT.x, goc_xDuong_yAm_zDuong_VT.y, goc_xDuong_yAm_zDuong_VT.z );
      
      printf( "%5.3f %5.3f\n", goc_xAm_yDuong_zAm_VT.x, goc_xAm_yDuong_zAm_VT.y, goc_xAm_yDuong_zAm_VT.z );
      printf( "%5.3f %5.3f\n", goc_xDuong_yDuong_zAm_VT.x, goc_xDuong_yDuong_zAm_VT.y, goc_xDuong_yDuong_zAm_VT.z );
      printf( "%5.3f %5.3f\n", goc_xAm_yDuong_zDuong_VT.x, goc_xAm_yDuong_zDuong_VT.y, goc_xAm_yDuong_zDuong_VT.z );
      printf( "%5.3f %5.3f\n\n", goc_xDuong_yDuong_zDuong_VT.x, goc_xDuong_yDuong_zDuong_VT.y, goc_xDuong_yDuong_zDuong_VT.z );
      
      printf( "phongTo %5.3f %5.3f %5.3f\n", vatThe->phongTo[0], vatThe->phongTo[5], vatThe->phongTo[10] );
      printf( "xoay %5.3f %5.3f %5.3f\n", vatThe->xoay[0], vatThe->xoay[5], vatThe->xoay[10] );
      printf( "dich %5.3f %5.3f %5.3f\n", vatThe->dich[12], vatThe->dich[13], vatThe->dich[14] );
      
         printf( "\n-----vatThe->nghichXoay\n" );
       printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichXoay[0], vatThe->nghichXoay[1], vatThe->nghichXoay[2], vatThe->nghichXoay[3] );
       printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichXoay[4], vatThe->nghichXoay[5], vatThe->nghichXoay[6], vatThe->nghichXoay[7] );
       printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichXoay[8], vatThe->nghichXoay[9], vatThe->nghichXoay[10], vatThe->nghichXoay[11] );
       printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichXoay[12], vatThe->nghichXoay[13], vatThe->nghichXoay[14], vatThe->nghichXoay[15] );
       printf( "\n-----vatThe->nghichBienHoa\n" );
       printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichBienHoa[0], vatThe->nghichBienHoa[1], vatThe->nghichBienHoa[2], vatThe->nghichBienHoa[3] );
       printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichBienHoa[4], vatThe->nghichBienHoa[5], vatThe->nghichBienHoa[6], vatThe->nghichBienHoa[7] );
       printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichBienHoa[8], vatThe->nghichBienHoa[9], vatThe->nghichBienHoa[10], vatThe->nghichBienHoa[11] );
       printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichBienHoa[12], vatThe->nghichBienHoa[13], vatThe->nghichBienHoa[14], vatThe->nghichBienHoa[15] ); */
      
      goc_xAm_yAm_zAm_TG = nhanVectoVoiMaTran4x4( &goc_xAm_yAm_zAm_VT, vatThe->bienHoa );
      goc_xDuong_yAm_zAm_TG = nhanVectoVoiMaTran4x4( &goc_xDuong_yAm_zAm_VT, vatThe->bienHoa );
      goc_xAm_yAm_zDuong_TG = nhanVectoVoiMaTran4x4( &goc_xAm_yAm_zDuong_VT, vatThe->bienHoa );
      goc_xDuong_yAm_zDuong_TG = nhanVectoVoiMaTran4x4( &goc_xDuong_yAm_zDuong_VT, vatThe->bienHoa );
      
      goc_xAm_yDuong_zAm_TG = nhanVectoVoiMaTran4x4( &goc_xAm_yDuong_zAm_VT, vatThe->bienHoa );
      goc_xDuong_yDuong_zAm_TG = nhanVectoVoiMaTran4x4( &goc_xDuong_yDuong_zAm_VT, vatThe->bienHoa );
      goc_xAm_yDuong_zDuong_TG = nhanVectoVoiMaTran4x4( &goc_xAm_yDuong_zDuong_VT, vatThe->bienHoa );
      goc_xDuong_yDuong_zDuong_TG = nhanVectoVoiMaTran4x4( &goc_xDuong_yDuong_zDuong_VT, vatThe->bienHoa );
/*
      printf( "%5.3f %5.3f\n", goc_xAm_yAm_zAm_TG.x, goc_xAm_yAm_zAm_TG.y, goc_xAm_yAm_zAm_TG.z );
      printf( "%5.3f %5.3f\n", goc_xDuong_yAm_zAm_TG.x, goc_xDuong_yAm_zAm_TG.y, goc_xAm_yDuong_zAm_TG.z );
      printf( "%5.3f %5.3f\n", goc_xAm_yAm_zDuong_TG.x, goc_xAm_yAm_zDuong_TG.y, goc_xAm_yAm_zDuong_TG.z );
      printf( "%5.3f %5.3f\n\n", goc_xDuong_yAm_zDuong_TG.x, goc_xDuong_yAm_zDuong_TG.y, goc_xDuong_yAm_zDuong_TG.z );
   
      printf( "%5.3f %5.3f\n", goc_xAm_yDuong_zAm_TG.x, goc_xAm_yDuong_zAm_TG.y, goc_xAm_yDuong_zAm_TG.z );
      printf( "%5.3f %5.3f\n", goc_xDuong_yDuong_zAm_TG.x, goc_xDuong_yDuong_zAm_TG.y, goc_xDuong_yDuong_zAm_TG.z );
      printf( "%5.3f %5.3f\n", goc_xAm_yDuong_zDuong_TG.x, goc_xAm_yDuong_zDuong_TG.y, goc_xAm_yDuong_zDuong_TG.z );
      printf( "%5.3f %5.3f\n", goc_xDuong_yDuong_zDuong_TG.x, goc_xDuong_yDuong_zDuong_TG.y, goc_xDuong_yDuong_zDuong_TG.z );
      */
      vatThe->baoBiTG.gocCucTieu.x = timNhoNhatCuaTamGiaTri( goc_xAm_yAm_zAm_TG.x, goc_xAm_yAm_zDuong_TG.x, goc_xDuong_yAm_zAm_TG.x, goc_xDuong_yAm_zDuong_TG.x,
                                                            goc_xAm_yDuong_zAm_TG.x, goc_xAm_yDuong_zDuong_TG.x, goc_xDuong_yDuong_zAm_TG.x, goc_xDuong_yDuong_zDuong_TG.x );
      vatThe->baoBiTG.gocCucTieu.y = timNhoNhatCuaTamGiaTri( goc_xAm_yAm_zAm_TG.y, goc_xAm_yAm_zDuong_TG.y, goc_xDuong_yAm_zAm_TG.y, goc_xDuong_yAm_zDuong_TG.y,
                                                            goc_xAm_yDuong_zAm_TG.y, goc_xAm_yDuong_zDuong_TG.y, goc_xDuong_yDuong_zAm_TG.y, goc_xDuong_yDuong_zDuong_TG.y );
      vatThe->baoBiTG.gocCucTieu.z = timNhoNhatCuaTamGiaTri( goc_xAm_yAm_zAm_TG.z, goc_xAm_yAm_zDuong_TG.z, goc_xDuong_yAm_zAm_TG.z, goc_xDuong_yAm_zDuong_TG.z,
                                                            goc_xAm_yDuong_zAm_TG.z, goc_xAm_yDuong_zDuong_TG.z, goc_xDuong_yDuong_zAm_TG.z, goc_xDuong_yDuong_zDuong_TG.z );
      
      
      vatThe->baoBiTG.gocCucDai.x = timLonNhatCuaTamGiaTri( goc_xAm_yAm_zAm_TG.x, goc_xAm_yAm_zDuong_TG.x, goc_xDuong_yAm_zAm_TG.x, goc_xDuong_yAm_zDuong_TG.x,
                                                           goc_xAm_yDuong_zAm_TG.x, goc_xAm_yDuong_zDuong_TG.x, goc_xDuong_yDuong_zAm_TG.x, goc_xDuong_yDuong_zDuong_TG.x );
      vatThe->baoBiTG.gocCucDai.y = timLonNhatCuaTamGiaTri( goc_xAm_yAm_zAm_TG.y, goc_xAm_yAm_zDuong_TG.y, goc_xDuong_yAm_zAm_TG.y, goc_xDuong_yAm_zDuong_TG.y,
                                                           goc_xAm_yDuong_zAm_TG.y, goc_xAm_yDuong_zDuong_TG.y, goc_xDuong_yDuong_zAm_TG.y, goc_xDuong_yDuong_zDuong_TG.y );
      vatThe->baoBiTG.gocCucDai.z = timLonNhatCuaTamGiaTri( goc_xAm_yAm_zAm_TG.z, goc_xAm_yAm_zDuong_TG.z, goc_xDuong_yAm_zAm_TG.z, goc_xDuong_yAm_zDuong_TG.z,
                                                           goc_xAm_yDuong_zAm_TG.z, goc_xAm_yDuong_zDuong_TG.z, goc_xDuong_yDuong_zAm_TG.z, goc_xDuong_yDuong_zDuong_TG.z );
   }

   
   
//   printf( "tinhBaoBi: %5.3f %5.3f %5.3f\n", vatThe->baoBiTG.gocCucDai.x - vatThe->baoBiTG.gocCucTieu.x, vatThe->baoBiTG.gocCucDai.y - vatThe->baoBiTG.gocCucTieu.y, vatThe->baoBiTG.gocCucDai.z - vatThe->baoBiTG.gocCucTieu.z );
   if( (vatThe->baoBiTG.gocCucDai.x - vatThe->baoBiTG.gocCucTieu.x == 0.0f) || (vatThe->baoBiTG.gocCucDai.y - vatThe->baoBiTG.gocCucTieu.y == 0.0f) || (vatThe->baoBiTG.gocCucDai.z - vatThe->baoBiTG.gocCucTieu.z == 0.0f) ) {
      printf( "SAI LẦM: Bao bì cỡ kích 0.0  loại %d  tâm %5.3f %5.3f %5.3f\n",  vatThe->loai, vatThe->dich[12], vatThe->dich[13], vatThe->dich[14] );
      printf( "baoBiVT   %5.3f %5.3f %5.3f    %5.3f %5.3f %5.3f\n", vatThe->baoBiVT.gocCucTieu.x, vatThe->baoBiVT.gocCucTieu.y, vatThe->baoBiVT.gocCucTieu.z, vatThe->baoBiVT.gocCucDai.x, vatThe->baoBiVT.gocCucDai.y, vatThe->baoBiVT.gocCucDai.z );
      exit(0);
   }
      

}

// Cũng xài cho vật thể bool, tương đối với vật thể này
void tinhBaoBiVTChoVatTheGhep( VatThe *vatThe ) {
   
   unsigned short soLuongVatThe = vatThe->soLuongVatThe;
   
   if( soLuongVatThe == 0 )
      printf( "TínhBaoBìTGChoVatTheGhep: Số lượng vật thể = 0\n" );
   else {
      // ---- đặt bao bì phim trường bằng bao bì vật thể đầu
      tinhBaoBiTGChoVatThe( &(vatThe->danhSachVatThe[0]) );
      BaoBi baoBiVatTheGhep = vatThe->danhSachVatThe[0].baoBiVT;
      
      // ----
      unsigned short chiSo = 1;
      while ( chiSo < soLuongVatThe ) {
         tinhBaoBiTGChoVatThe( &(vatThe->danhSachVatThe[chiSo]) );
         
         BaoBi baoBiVT = vatThe->danhSachVatThe[chiSo].baoBiTG;
         
         // ---- X
         if( baoBiVT.gocCucTieu.x < baoBiVatTheGhep.gocCucTieu.x )
            baoBiVatTheGhep.gocCucTieu.x = baoBiVT.gocCucTieu.x;
         if( baoBiVT.gocCucDai.x > baoBiVatTheGhep.gocCucDai.x )
            baoBiVatTheGhep.gocCucDai.x = baoBiVT.gocCucDai.x;
         // ---- Y
         if( baoBiVT.gocCucTieu.y < baoBiVatTheGhep.gocCucTieu.y )
            baoBiVatTheGhep.gocCucTieu.y = baoBiVT.gocCucTieu.y;
         if( baoBiVT.gocCucDai.y > baoBiVatTheGhep.gocCucDai.y )
            baoBiVatTheGhep.gocCucDai.y = baoBiVT.gocCucDai.y;
         // ---- Z
         if( baoBiVT.gocCucTieu.z < baoBiVatTheGhep.gocCucTieu.z )
            baoBiVatTheGhep.gocCucTieu.z = baoBiVT.gocCucTieu.z;
         if( baoBiVT.gocCucDai.z > baoBiVatTheGhep.gocCucDai.z )
            baoBiVatTheGhep.gocCucDai.z = baoBiVT.gocCucDai.z;
         chiSo++;
      }
      
      vatThe->baoBiVT.gocCucTieu = baoBiVatTheGhep.gocCucTieu;
      vatThe->baoBiVT.gocCucDai = baoBiVatTheGhep.gocCucDai;
   }
}


float timNhoNhatCuaTamGiaTri( float so0, float so1, float so2, float so3, float so4, float so5, float so6, float so7 ) {
   
   float nhoNhat = so0;
   if( so1 < nhoNhat )
      nhoNhat = so1;
   if( so2 < nhoNhat )
      nhoNhat = so2;
   if( so3 < nhoNhat )
      nhoNhat = so3;
   if( so4 < nhoNhat )
      nhoNhat = so4;
   if( so5 < nhoNhat )
      nhoNhat = so5;
   if( so6 < nhoNhat )
      nhoNhat = so6;
   if( so7 < nhoNhat )
      nhoNhat = so7;
   
   return nhoNhat;
}

float timLonNhatCuaTamGiaTri( float so0, float so1, float so2, float so3, float so4, float so5, float so6, float so7 ) {
   
   float lonNhat = so0;
   if( so1 > lonNhat )
      lonNhat = so1;
   if( so2 > lonNhat )
      lonNhat = so2;
   if( so3 > lonNhat )
      lonNhat = so3;
   if( so4 > lonNhat )
      lonNhat = so4;
   if( so5 > lonNhat )
      lonNhat = so5;
   if( so6 > lonNhat )
      lonNhat = so6;
   if( so7 > lonNhat )
      lonNhat = so7;
   
   return lonNhat;
}

#pragma mark ---- Chia Thành Tầng Bậc
void datLaiMangChiVatThe( unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVatThe ) {
   
   unsigned short chiSo = 0;
   while ( chiSo < soLuongVatThe ) {
      mangChiSoVatTheSapXep[chiSo] = chiSo;
      chiSo++;
   }

}

unsigned int chiaVatThe( VatThe *danhSachVatThe, unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVatThe, unsigned short chiSoVatThe, BaoBi *baoBi, unsigned int truc, float toaDoMatPhang ) {

   // ---- giữ chỉ số cho giao điểm này
   unsigned int chiSoGiaoDiemNay = chiSoGiaoDiem;

   // ---- tính bao bì trại và phải
   if( soLuongVatThe > 1 ) {

      unsigned short soLuongTrai = 0;
      unsigned short soLuongPhai = 0;
      // ---- đặt giao điểm
      mangGiaoDiem[chiSoGiaoDiemNay] = chiaVatTheVaTaoGiaoDiem( danhSachVatThe, mangChiSoVatTheSapXep, soLuongVatThe, truc, toaDoMatPhang, &soLuongTrai, &soLuongPhai );
//   printf( "chiSoGiaoDiemNay %d  soLuongTrai %d  soLuongPhai %d\n", chiSoGiaoDiemNay, soLuongTrai, soLuongPhai );
      // ---- đã tạo giao điểm này, tăng lên cho giao điểm tiếp trong mảngGiaoĐiểm
      chiSoGiaoDiem++;
      if( chiSoGiaoDiem == kSO_LUONG__GIAO_DIEM_TOI_DA ) {
         printf( "chiSoaGiamDiem > kSO_LUONG__GIAO_DIEM_TOI_DA (%d)\n", kSO_LUONG__GIAO_DIEM_TOI_DA );
         exit(0);
      }

      // ---- chia bao bì thanh hai phần bằng nhau
      BaoBi baoBiTrai;
      BaoBi baoBiPhai;
      // ---- kiếm trục chia, xài cạnh dài nhất
      chiaBaoBi( truc, toaDoMatPhang, baoBi, &baoBiTrai, &baoBiPhai );

      // ---- nếu có vật thể trong bao bi trái
      if( soLuongTrai > 0 ) {
         tinhTrucVaToaMatPhangChia( &baoBiTrai, &truc, &toaDoMatPhang );
         mangGiaoDiem[chiSoGiaoDiemNay].conTrai = chiaVatThe( danhSachVatThe, &(mangChiSoVatTheSapXep[0]), soLuongTrai, chiSoVatThe, &baoBiTrai, truc, toaDoMatPhang );
      }
      else {
         mangGiaoDiem[chiSoGiaoDiemNay].conTrai = -1;
      }

      // ---- nếu có vật thể trong bao bi trái
      if( soLuongPhai > 0 ) {
         tinhTrucVaToaMatPhangChia( &baoBiPhai, &truc, &toaDoMatPhang );   // phải làm vì tọaĐộMặtPhẳng không giống
         mangGiaoDiem[chiSoGiaoDiemNay].conPhai = chiaVatThe( danhSachVatThe, &(mangChiSoVatTheSapXep[soLuongTrai]), soLuongPhai, chiSoVatThe + soLuongTrai, &baoBiPhai, truc, toaDoMatPhang );
      }
      else {
         mangGiaoDiem[chiSoGiaoDiemNay].conPhai = -1;
      }
   }
   else if( soLuongVatThe == 1) {
      mangGiaoDiem[chiSoGiaoDiemNay].truc = kLA;
      mangGiaoDiem[chiSoGiaoDiemNay].conTrai = -1;
      mangGiaoDiem[chiSoGiaoDiemNay].conPhai = -1;
      mangGiaoDiem[chiSoGiaoDiemNay].chiSoVatThe = chiSoVatThe;
      chiSoGiaoDiem++;
      if( chiSoGiaoDiem == kSO_LUONG__GIAO_DIEM_TOI_DA ) {
         printf( "chiSoaGiamDiem == kSO_LUONG__GIAO_DIEM_TOI_DA (%d)\n", kSO_LUONG__GIAO_DIEM_TOI_DA );
         exit(0);
      }
   }

   return chiSoGiaoDiemNay;
}


GiaoDiemBIH chiaVatTheVaTaoGiaoDiem( VatThe *danhSachVatThe, unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVatThe, unsigned int truc,
                                    float toaDoMatPhang, unsigned short *soLuongTrai, unsigned short *soLuongPhai ) {

   float cucTrai = -kVO_CUC;
   float cucPhai = kVO_CUC;

   short chiSoDau = 0;
   short chiSoCuoi = soLuongVatThe - 1;

   while( chiSoDau <= chiSoCuoi ) {

      BaoBi baoBiTG = danhSachVatThe[mangChiSoVatTheSapXep[chiSoDau]].baoBiTG;
//      printf( "%d baoBiTG: %5.3f %5.3f %5.3f  %5.3f %5.3f %5.3f  %d\n", mangChiSoVatTheSapXep[chiSoDau], baoBiTG.gocCucTieu.x, baoBiTG.gocCucTieu.y, baoBiTG.gocCucTieu.z,
//             baoBiTG.gocCucDai.x, baoBiTG.gocCucDai.y, baoBiTG.gocCucDai.z, danhSachVatThe[mangChiSoVatTheSapXep[chiSoDau]].loai );

      // ---- xem nếu vật thể ở bên trái hay bên phải
      if( truc == kTRUC_X ) {
         if( baoBiTG.gocCucDai.x < toaDoMatPhang ) {   // bên trái
            (*soLuongTrai)++;  // tăng số lượng vật thể bên trái
            if( baoBiTG.gocCucDai.x > cucTrai )
               cucTrai = baoBiTG.gocCucDai.x;
            chiSoDau++;  // không cần đổi vật thể
         }
         else if( baoBiTG.gocCucTieu.x > toaDoMatPhang ) {  // bên phải
            (*soLuongPhai)++;  // tăng số lượng vật thể bên phái
            if( baoBiTG.gocCucTieu.x < cucPhai )
               cucPhai = baoBiTG.gocCucTieu.x;
            // ---- đổi vật thể
            unsigned short chiSoVatThe = mangChiSoVatTheSapXep[chiSoDau];
            mangChiSoVatTheSapXep[chiSoDau] = mangChiSoVatTheSapXep[chiSoCuoi];
            mangChiSoVatTheSapXep[chiSoCuoi] = chiSoVatThe;
            chiSoCuoi--;
         }
         else {   // vật thể nằm trên mặt phẳng chia, cần xem ở bên nào nhất
            float cachTrai = toaDoMatPhang - baoBiTG.gocCucTieu.x;
            float cachPhai = baoBiTG.gocCucDai.x - toaDoMatPhang;
            if( cachTrai > cachPhai ) {
               (*soLuongTrai)++;  // tăng số lượng vật thể bên trái
               if( baoBiTG.gocCucDai.x > cucTrai )
                  cucTrai = baoBiTG.gocCucDai.x;
               chiSoDau++;
            }
            else {
               (*soLuongPhai)++;  // tăng số lượng vật thể bên phái
               if( baoBiTG.gocCucTieu.x < cucPhai )
                  cucPhai = baoBiTG.gocCucTieu.x;
               // ---- đổi vật thể
               unsigned short chiSoVatThe = mangChiSoVatTheSapXep[chiSoDau];
               mangChiSoVatTheSapXep[chiSoDau] = mangChiSoVatTheSapXep[chiSoCuoi];
               mangChiSoVatTheSapXep[chiSoCuoi] = chiSoVatThe;
               chiSoCuoi--;
            }
         }
      }
      else if( truc == kTRUC_Y ) {
         if( baoBiTG.gocCucDai.y < toaDoMatPhang ) {   // bên trái
            (*soLuongTrai)++;  // tăng số lượng vật thể bên trái
            if( baoBiTG.gocCucDai.y > cucTrai )
               cucTrai = baoBiTG.gocCucDai.y;
            chiSoDau++;  // không cần đổi vật thể
         }
         else if( baoBiTG.gocCucTieu.y > toaDoMatPhang ) {  // bên phải
            (*soLuongPhai)++;  // tăng số lượng vật thể bên phái
            if( baoBiTG.gocCucTieu.y < cucPhai )
               cucPhai = baoBiTG.gocCucTieu.y;
            // ---- đổi vật thể
            unsigned short chiSoVatThe = mangChiSoVatTheSapXep[chiSoDau];
            mangChiSoVatTheSapXep[chiSoDau] = mangChiSoVatTheSapXep[chiSoCuoi];
            mangChiSoVatTheSapXep[chiSoCuoi] = chiSoVatThe;
            chiSoCuoi--;
         }
         else {   // vật thể nằm trên mặt phẳng chia, cần xem ở bên nào nhất
            float cachTrai = toaDoMatPhang - baoBiTG.gocCucTieu.y;
            float cachPhai = baoBiTG.gocCucDai.y - toaDoMatPhang;
            if( cachTrai > cachPhai ) {
               (*soLuongTrai)++;  // tăng số lượng vật thể bên trái
               if( baoBiTG.gocCucDai.y > cucTrai )
                  cucTrai = baoBiTG.gocCucDai.y;
               chiSoDau++;
            }
            else {
               (*soLuongPhai)++;  // tăng số lượng vật thể bên phái
               if( baoBiTG.gocCucTieu.y < cucPhai )
                  cucPhai = baoBiTG.gocCucTieu.y;
               // ---- đổi vật thể
               unsigned short chiSoVatThe = mangChiSoVatTheSapXep[chiSoDau];
               mangChiSoVatTheSapXep[chiSoDau] = mangChiSoVatTheSapXep[chiSoCuoi];
               mangChiSoVatTheSapXep[chiSoCuoi] = chiSoVatThe;
               chiSoCuoi--;
            }
         }
      }
      else {//if( truc == kTRUC_Z ) {
         if( baoBiTG.gocCucDai.z < toaDoMatPhang ) {   // bên trái
            (*soLuongTrai)++;  // tăng số lượng vật thể bên trái
            if( baoBiTG.gocCucDai.z > cucTrai )
               cucTrai = baoBiTG.gocCucDai.z;
            chiSoDau++;  // không cần đổi vật thể
         }
         else if( baoBiTG.gocCucTieu.z > toaDoMatPhang ) {  // bên phải
            (*soLuongPhai)++;  // tăng số lượng vật thể bên phái
            if( baoBiTG.gocCucTieu.z < cucPhai )
               cucPhai = baoBiTG.gocCucTieu.z;
            // ---- đổi vật thể
            unsigned short chiSoVatThe = mangChiSoVatTheSapXep[chiSoDau];
            mangChiSoVatTheSapXep[chiSoDau] = mangChiSoVatTheSapXep[chiSoCuoi];
            mangChiSoVatTheSapXep[chiSoCuoi] = chiSoVatThe;
            chiSoCuoi--;
         }
         else {   // vật thể nằm trên mặt phẳng chia, cần xem ở bên nào nhất
            float cachTrai = toaDoMatPhang - baoBiTG.gocCucTieu.z;
            float cachPhai = baoBiTG.gocCucDai.z - toaDoMatPhang;
            if( cachTrai > cachPhai ) {
               (*soLuongTrai)++;  // tăng số lượng vật thể bên trái
               if( baoBiTG.gocCucDai.z > cucTrai )
                  cucTrai = baoBiTG.gocCucDai.z;
               chiSoDau++;
            }
            else {  // tăng số lượng vật thể bên phái
               (*soLuongPhai)++;
               if( baoBiTG.gocCucTieu.z < cucPhai )
                  cucPhai = baoBiTG.gocCucTieu.z;
               // ---- đổi vật thể
               unsigned short chiSoVatThe = mangChiSoVatTheSapXep[chiSoDau];
               mangChiSoVatTheSapXep[chiSoDau] = mangChiSoVatTheSapXep[chiSoCuoi];
               mangChiSoVatTheSapXep[chiSoCuoi] = chiSoVatThe;
               chiSoCuoi--;
            }
         }
      }
   }

   
   // ---- tạo giao điểm
   GiaoDiemBIH giaoDiem;

   giaoDiem.cat[0] = cucTrai;
   giaoDiem.cat[1] = cucPhai;
   giaoDiem.truc = truc;
   
   return giaoDiem;
}

void tinhTrucVaToaMatPhangChia( BaoBi *baoBi, unsigned int *truc, float *toaDoMatPhang ) {
   // ---- kiếm trục chia, xài cạnh dài nhất
   float canh = baoBi->gocCucDai.x - baoBi->gocCucTieu.x;
   *truc = kTRUC_X;
   float canhY = baoBi->gocCucDai.y - baoBi->gocCucTieu.y;
   if( canh < canhY ) {
      canh = canhY;
      *truc = kTRUC_Y;
   }
   float canhZ = baoBi->gocCucDai.z - baoBi->gocCucTieu.z;
   if( canh < canhZ ) {
      canh = canhZ;
      *truc = kTRUC_Z;
   }
   
   // ---- tính tọa độ mặt phẳng, phần nữa cạnh dài nhất
   if( *truc == kTRUC_X )
      *toaDoMatPhang = baoBi->gocCucTieu.x + canh*0.5f;
   else if( *truc == kTRUC_Y )
      *toaDoMatPhang = baoBi->gocCucTieu.y + canh*0.5f;
   else  // if( *truc == kTRUC_Z )
      *toaDoMatPhang = baoBi->gocCucTieu.z + canh*0.5f;
   
/*   printf( "BaoBi %5.3f %5.3f %5.3f  %5.3f %5.3f %5.3f  Truc %x  toaDoMatPhang %5.3f\n", baoBi->gocCucTieu.x, baoBi->gocCucTieu.y, baoBi->gocCucTieu.z,
          baoBi->gocCucDai.x, baoBi->gocCucDai.y, baoBi->gocCucDai.z,
          *truc, *toaDoMatPhang ); */
}

void chiaBaoBi( unsigned int truc, float matPhangChia, BaoBi *baoBi, BaoBi *baoBiTrai, BaoBi *baoBiPhai ) {
   
   // ---- chép bao bì
   baoBiTrai->gocCucTieu.x = baoBi->gocCucTieu.x;
   baoBiTrai->gocCucTieu.y = baoBi->gocCucTieu.y;
   baoBiTrai->gocCucTieu.z = baoBi->gocCucTieu.z;
   baoBiTrai->gocCucDai.x = baoBi->gocCucDai.x;
   baoBiTrai->gocCucDai.y = baoBi->gocCucDai.y;
   baoBiTrai->gocCucDai.z = baoBi->gocCucDai.z;
   
   baoBiPhai->gocCucTieu.x = baoBi->gocCucTieu.x;
   baoBiPhai->gocCucTieu.y = baoBi->gocCucTieu.y;
   baoBiPhai->gocCucTieu.z = baoBi->gocCucTieu.z;
   baoBiPhai->gocCucDai.x = baoBi->gocCucDai.x;
   baoBiPhai->gocCucDai.y = baoBi->gocCucDai.y;
   baoBiPhai->gocCucDai.z = baoBi->gocCucDai.z;
   
   // ---- tính bao bì chia mới
   if( truc == kTRUC_X ) {
      baoBiTrai->gocCucDai.x = matPhangChia;
      baoBiPhai->gocCucTieu.x = matPhangChia;
   }
   else if( truc == kTRUC_Y ) {
      baoBiTrai->gocCucDai.y = matPhangChia;
      baoBiPhai->gocCucTieu.y = matPhangChia;
   }
   else { //if( truc == kTRUC_Z ) {
      baoBiTrai->gocCucDai.z = matPhangChia;
      baoBiPhai->gocCucTieu.z = matPhangChia;
   }
}

#pragma mark ---- Xem Vật Thể
void xemCay( GiaoDiemBIH *cay, unsigned short soLuongGiaoDiem ) {
   
   unsigned short chiSo = 0;
   while( chiSo < soLuongGiaoDiem ) {
      GiaoDiemBIH giaoDiem = cay[chiSo];
      if( giaoDiem.truc == kLA )
         printf( "%d Lá VậtThể: %d\n", chiSo, giaoDiem.chiSoVatThe );
      else
         printf( "%d Giao Điểm trai %d (%5.3f)  phai %d (%5.3f) truc %x\n", chiSo, giaoDiem.conTrai, giaoDiem.cat[0], giaoDiem.conPhai, giaoDiem.cat[1], giaoDiem.truc );

      chiSo++;
   }
}

void xemVatThe( VatThe *danhSachVatThe, unsigned short soLuongVatThe ) {
   
   unsigned short chiSo = 0;
   while( chiSo < soLuongVatThe ) {
      VatThe vatThe = danhSachVatThe[chiSo];
      printf( "%d loai %d tâm %5.3f %5.3f %5.3f\n", chiSo, vatThe.loai, vatThe.bienHoa[12], vatThe.bienHoa[13], vatThe.bienHoa[14] );
      printf( "     quat %5.3f %5.3f %5.3f %5.3f\n", vatThe.quaternion.w, vatThe.quaternion.x, vatThe.quaternion.y, vatThe.quaternion.z );
      printf( "     phongTo %5.3f %5.3f %5.3f\n", vatThe.phongTo[0], vatThe.phongTo[5], vatThe.phongTo[10] );
      printf( "     dich %5.3f %5.3f %5.3f  %5.3f %5.3f %5.3f\n", vatThe.dich[0], vatThe.dich[5], vatThe.dich[10], vatThe.dich[12], vatThe.dich[13],
             vatThe.dich[14] );
      
      chiSo++;
   }
}

#pragma mark ---- Tìm Có Trúng
unsigned char thayNguonAnhSang( VatThe *danhSachVat, unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVat, Vecto *diemTrung, Vecto *huongAnhSang, ThongTinToMau *thongTinToMauBong, unsigned char *soLuongVatTheToMauBong ) {

   Tia tiaDenNguonaAnhSang;

   // ---- hướng đến ngườn ảnh sáng là ngược hướng tia ánh sáng
   tiaDenNguonaAnhSang.huong.x = -huongAnhSang->x;
   tiaDenNguonaAnhSang.huong.y = -huongAnhSang->y;
   tiaDenNguonaAnhSang.huong.z = -huongAnhSang->z;
   
   tiaDenNguonaAnhSang.goc.x = diemTrung->x;// + 0.001f*tiaDenNguonaAnhSang.huong.x;
   tiaDenNguonaAnhSang.goc.y = diemTrung->y;// + 0.001f*tiaDenNguonaAnhSang.huong.y;
   tiaDenNguonaAnhSang.goc.z = diemTrung->z;// + 0.001f*tiaDenNguonaAnhSang.huong.z;

   // ==== xem nếu tia có trúng vật thể nào
   // ---- xem nếu có trúng vật thể nào
   unsigned char ketQuaThay = xemTiaCoTrungVatTheNao( mangGiaoDiem, chiSoGiaoDiem, danhSachVat, mangChiSoVatTheSapXep, &(tiaDenNguonaAnhSang), thongTinToMauBong, soLuongVatTheToMauBong );
//   printf( "-- thayNguon: thay %d phapTuyen %5.3f %5.3f %5.3f\n", ketQuaThay, thongTinToMauBong->phapTuyenTDVT.x, thongTinToMauBong->phapTuyenTDVT.y, thongTinToMauBong->phapTuyenTDVT.z );
   return !ketQuaThay;
}

typedef struct {  // đang làm chưa xong nhe!
   Mau mauVat;
   float cach;
} DoanBongToi;

unsigned char tinhAnhSangBongTuNguonAnhSang( VatThe *danhSachVat, unsigned short *mangChiSoVatTheSapXep, unsigned short soLuongVat, Vecto *diemTrung, Vecto *huongAnhSang ) {
   
   Tia tiaDenNguonaAnhSang;
   
   // ---- hướng đến ngườn ảnh sáng là ngược hướng tia ánh sáng
   tiaDenNguonaAnhSang.huong.x = -huongAnhSang->x;
   tiaDenNguonaAnhSang.huong.y = -huongAnhSang->y;
   tiaDenNguonaAnhSang.huong.z = -huongAnhSang->z;
   
   tiaDenNguonaAnhSang.goc.x = diemTrung->x;
   tiaDenNguonaAnhSang.goc.y = diemTrung->y;
   tiaDenNguonaAnhSang.goc.z = diemTrung->z;
   
   DoanBongToi mangBongToi[32];   // mảng bong tốo, danh sách các vật thể tia được trúng trên tia đến nguồn ánh sáng
   unsigned char soDoan = 0;

   // ==== xem nếu tia có trúng vật thể nào
      // ---- trúng vật thể, tìm cách xa trong vật thể
      // ---- giữ màu và độ đục của vật thể
      // ---- tính điểm trúng khi ra vật thể và 
   return kDUNG;
}

#pragma mark ---- Tô Màu Hoạ Tiết
Mau toMauVat( VatThe *vatThe, Vecto *diemTrungTDVT, Vecto phapTuyen, Vecto huongTia ) {

//   printf( "  toMauVat: phapTuyen %5.3f %5.3f %5.3f\n", phapTuyen.x, phapTuyen.y, phapTuyen.z );
   // ---- điểm trúng biến hóa rồi (tọa độ vật thể)
   Mau mauVat;
   if( vatThe->soHoaTiet == kHOA_TIET__KHONG )
      mauVat = vatThe->hoaTiet.hoaTietKhong.mau;
   
   else if( vatThe->soHoaTiet == kHOA_TIET__DI_HUONG )
      mauVat = hoaTietDiHuong( phapTuyen, huongTia, &(vatThe->hoaTiet.hoaTietDiHuong) );

   else if( vatThe->soHoaTiet == kHOA_TIET__CA_RO )
      mauVat = hoaTietCaRo( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietCaRo) );

   else if( vatThe->soHoaTiet == kHOA_TIET__VONG_TRON )
      mauVat = hoaTietVongTron( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietVongTron) );

   else if( vatThe->soHoaTiet == kHOA_TIET__OC_XOAY )
      mauVat = hoaTietOcXoay( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietOcXoay) );

   else if( vatThe->soHoaTiet == kHOA_TIET__TRAI_BANH )

      mauVat = hoaTietTraiBanh( diemTrungTDVT );

   else if( vatThe->soHoaTiet == kHOA_TIET__GAN )
      mauVat = hoaTietGan( diemTrungTDVT->x, &(vatThe->hoaTiet.hoaTietGan) );

   else if( vatThe->soHoaTiet == kHOA_TIET__CHAM_BI )
      mauVat = hoaTietChamBi( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietChamBi) );
   
   else if( vatThe->soHoaTiet == kHOA_TIET__NGOI_SAO_CAU )
      mauVat = hoaTietNgoiSaoCau( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietNgoiSaoCau) );

   else if( vatThe->soHoaTiet == kHOA_TIET__QUAN )
      mauVat = hoaTietQuan( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietQuan) );

   else if( vatThe->soHoaTiet == kHOA_TIET__QUAN_XOAY )
      mauVat = hoaTietQuanXoay( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietQuanXoay) );

   else if( vatThe->soHoaTiet == kHOA_TIET__QUAN_SONG_THEO_HUONG )
      mauVat = hoaTietQuanSongTheoHuong( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietQuanSongTheoHuong) );

   else if( vatThe->soHoaTiet == kHOA_TIET__QUAN_SONG_TRUC_Z )
      mauVat = hoaTietQuanSongTrucZ( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietQuanSongTrucZ) );

   else if( vatThe->soHoaTiet == kHOA_TIET__QUAN_SONG_TIA_PHAI )
      mauVat = hoaTietQuanSongTiaPhai( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietQuanSongTiaPhai) );

   else if( vatThe->soHoaTiet == kHOA_TIET__SOC )
      mauVat = hoaTietSoc( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietSoc) );

   else if( vatThe->soHoaTiet == kHOA_TIET__CA_RO_MIN )
      mauVat = hoaTietCaRoMin( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietCaRoMin) );

   else if( vatThe->soHoaTiet == kHOA_TIET__HAI_CHAM_BI )
      mauVat = hoaTietHaiChamBi( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietHaiChamBi) );

   else if( vatThe->soHoaTiet == kHOA_TIET__BONG_VONG )
      mauVat = hoaTietBongVong( diemTrungTDVT, &(vatThe->hoaTiet.hoaTietBongVong) );
   
   else
      printf( "ToMâuVật: không biết họa tiết này %d\n", vatThe->soHoaTiet );
//   printf( "toMauVat: phapTuyen %5.3f %5.3f %5.3f   mauVat.dd %5.3f\n", phapTuyen.x, phapTuyen.y, phapTuyen.z, mauVat.dd );
   return mauVat;
}


// thiên đỉnh
//
// chân trời
//
// thiên đấy
Mau tinhMauTroi( Vecto *huongTia ) {
   
   Mau mauChanTroi;   // màu chân trời
   mauChanTroi.d = 0.7f;
   mauChanTroi.l = 0.7f;
   mauChanTroi.x = 1.0f;
   mauChanTroi.dd = 1.0f;

   Mau mauThienDinh;   // màu thiên đỉnh
   mauThienDinh.d = 0.0f;
   mauThienDinh.l = 0.0f;
   mauThienDinh.x = 0.5f;
   mauThienDinh.dd = 1.0f;
   
//   Mau mauThienDay;   // màu thiên đấy
//   mauThienDay.d = 0.5f;
//   mauThienDay.l = 0.2f;
//   mauThienDay.x = 0.0f;
//   mauThienDay.dd = 1.0f;
   
   // ---- chép hướng tia, không muốn đơn vị họa hướngTia gốc
   Vecto huongTiaDVH;  // hướngTiaĐơnVịHóa 
   huongTiaDVH.x = huongTia->x;
   huongTiaDVH.y = huongTia->y;
   huongTiaDVH.z = huongTia->z;

   donViHoa( &huongTiaDVH );

   Mau mauTroi;
   if( huongTiaDVH.y < 0.0f ) {
      mauTroi.d = mauChanTroi.d;
      mauTroi.l = mauChanTroi.l;
      mauTroi.x = mauChanTroi.x;
      mauTroi.dd = mauChanTroi.dd;
   }
   else {   // bầu trời
      float tiSoChanTroi = 1.0f - huongTiaDVH.y;
//      printf( "huongY %5.3f  tiSoChanTroi %5.3f\n", huongTiaDVH.y, tiSoChanTroi );
      tiSoChanTroi = powf( tiSoChanTroi, 8.0f );
//      printf( "    tiSoChanTroi %5.3f\n", tiSoChanTroi );
      mauTroi.d = mauChanTroi.d*tiSoChanTroi + mauThienDinh.d*(1.0f - tiSoChanTroi);
      mauTroi.l = mauChanTroi.l*tiSoChanTroi + mauThienDinh.l*(1.0f - tiSoChanTroi);
      mauTroi.x = mauChanTroi.x*tiSoChanTroi + mauThienDinh.x*(1.0f - tiSoChanTroi);
      mauTroi.dd = mauChanTroi.dd*tiSoChanTroi + mauThienDinh.dd*(1.0f - tiSoChanTroi);
   }

   return mauTroi;
}


Mau tinhMauTanXa( Mau *mauMatTroi, Mau *mauVat, Vecto *huongNguonAnhSang, Vecto *phapTuyen ) {
   
   // ---- tính độ sáng tử mặt
   float tichVoHuong = huongNguonAnhSang->x*phapTuyen->x + huongNguonAnhSang->y*phapTuyen->y + huongNguonAnhSang->z*phapTuyen->z;

   // ---- ánh sáng trúng mặt có hướng nghịch chiếu với pháp tuyến
   tichVoHuong = -tichVoHuong;
   
   if( tichVoHuong < 0.0f )
      tichVoHuong = 0.0f;  // cho một chút ánh sáng để giả bộ ánh sáng từ môi trường

   Mau mauTanXa;
   mauTanXa.d = tichVoHuong*(mauVat->d);
   mauTanXa.l = tichVoHuong*(mauVat->l);
   mauTanXa.x = tichVoHuong*(mauVat->x);
   mauTanXa.dd = tichVoHuong*(mauVat->dd);
   
   mauTanXa.d *= mauMatTroi->d;
   mauTanXa.l *= mauMatTroi->l;
   mauTanXa.x *= mauMatTroi->x;
//   mauTanXa.dd *= 1.0f;//mauMatTroi->dd;
   
   mauTanXa.d += 0.05f*mauVat->d;
   mauTanXa.l += 0.05f*mauVat->l;
   mauTanXa.x += 0.05f*mauVat->x;
   return mauTanXa;
}

Tia tinhTiaPhanXa( Vecto phapTuyen, Vecto diemTrung, Vecto huongTrung ) {
   
   Tia tiaPhanXa;

   // ---- tích vô hướng giữa hướng tia gốc với vectơ vuông góc
   donViHoa( &huongTrung );
   float tichVoHuong = phapTuyen.x*huongTrung.x + phapTuyen.y*huongTrung.y + phapTuyen.z*huongTrung.z;

   // ---- tính hướng tia phản xạ
   tiaPhanXa.huong.x = huongTrung.x - 2.0f*tichVoHuong*phapTuyen.x;
   tiaPhanXa.huong.y = huongTrung.y - 2.0f*tichVoHuong*phapTuyen.y;
   tiaPhanXa.huong.z = huongTrung.z - 2.0f*tichVoHuong*phapTuyen.z;
   
   // ---- cộng thêm một chút cho ra vật thể
   tiaPhanXa.goc.x = diemTrung.x + 0.001f*tiaPhanXa.huong.x;
   tiaPhanXa.goc.y = diemTrung.y + 0.001f*tiaPhanXa.huong.y;
   tiaPhanXa.goc.z = diemTrung.z + 0.001f*tiaPhanXa.huong.z;

   return tiaPhanXa;
}

Tia tinhTiaKhucXa( Vecto phapTuyen, Vecto diemTrung, Vecto huongTrung, float chietSuat ) {

   Tia tiaKhucXa;

   // ---- tia trúng cùng hướng với pháp tuyến cho biết hướng khúc xạ
   donViHoa( &huongTrung );
   float tichVoHuong = phapTuyen.x*huongTrung.x + phapTuyen.y*huongTrung.y + phapTuyen.z*huongTrung.z;

   if( tichVoHuong < 0.0f ) { // vào vật thể
      float tiSoXuyen = 1.0f/chietSuat; // sin θ2 - tỉ số chiết suất
      float a = 1.0f - tichVoHuong*tichVoHuong;   // nhớ tích vô hướng < 0.0
      float b = sqrtf( 1.0f - tiSoXuyen*tiSoXuyen*a );  // cos θ2
      
      tiaKhucXa.huong.x = (huongTrung.x - phapTuyen.x*tichVoHuong)*tiSoXuyen - phapTuyen.x*b;
      tiaKhucXa.huong.y = (huongTrung.y - phapTuyen.y*tichVoHuong)*tiSoXuyen - phapTuyen.y*b;
      tiaKhucXa.huong.z = (huongTrung.z - phapTuyen.z*tichVoHuong)*tiSoXuyen - phapTuyen.z*b;

      // ---- cộng thêm một chút cho vào vật thể
      tiaKhucXa.goc.x = diemTrung.x + 0.001f*tiaKhucXa.huong.x;
      tiaKhucXa.goc.y = diemTrung.y + 0.001f*tiaKhucXa.huong.y;
      tiaKhucXa.goc.z = diemTrung.z + 0.001f*tiaKhucXa.huong.z;
   }
   else { // ra chất liệu
      float tiSoXuyen = chietSuat/1.0f;   // sin θ2 - tỉ số chiết suất
      float a = 1.0f - tichVoHuong*tichVoHuong;   // nhớ tích vô hướng < 0.0
      float b = 1.0f - tiSoXuyen*tiSoXuyen*a;  // cos θ2
      if( b < 0.0f ) {  // θ2 > 90º, không khúc xạ, phản xạ
         tiaKhucXa = tinhTiaPhanXa( phapTuyen, diemTrung, huongTrung );
      }
      else {
         b = sqrtf(b);
         tiaKhucXa.huong.x = (huongTrung.x - phapTuyen.x*tichVoHuong)*tiSoXuyen + phapTuyen.x*b;
         tiaKhucXa.huong.y = (huongTrung.y - phapTuyen.y*tichVoHuong)*tiSoXuyen + phapTuyen.y*b;
         tiaKhucXa.huong.z = (huongTrung.z - phapTuyen.z*tichVoHuong)*tiSoXuyen + phapTuyen.z*b;

         // ---- cộng thêm một chút cho ra vật thể
         tiaKhucXa.goc.x = diemTrung.x + 0.001f*tiaKhucXa.huong.x;
         tiaKhucXa.goc.y = diemTrung.y + 0.001f*tiaKhucXa.huong.y;
         tiaKhucXa.goc.z = diemTrung.z + 0.001f*tiaKhucXa.huong.z;
      }
   }


   return tiaKhucXa;
}

Mau tinhCaoQuang( MatTroi *matTroi, Vecto *huongPhanXa, float mu ) {
   
   Vecto *huongAnhSang = &(matTroi->huongAnh);
   float doSang = huongAnhSang->x*huongPhanXa->x + huongAnhSang->y*huongPhanXa->y + huongAnhSang->z*huongPhanXa->z;
   
   // ---- hướng ánh sáng nên nghích chiếu với pháp tuyến
   doSang = -doSang;

   if( doSang < 0.0f )
      doSang = 0.0f;
   else
      doSang = powf( doSang, mu );

   Mau mauCaoQuang;
   mauCaoQuang.d = doSang*matTroi->mauAnh.d;
   mauCaoQuang.l = doSang*matTroi->mauAnh.l;
   mauCaoQuang.x = doSang*matTroi->mauAnh.x;
   mauCaoQuang.dd = doSang*matTroi->mauAnh.dd;
   
   return mauCaoQuang;
}


Mau toSuongMu( Vecto *huongTia, Mau *mauVat, Mau *mauTroi ) {

   Mau ketQua;
   // ---- chỉ tô màu vật thể nếu xa hơn 10.0f
   if( (mauVat->c > 10.0f) && (mauVat->c < kVO_CUC) ) {
      float tiSo = expf( (10.0f - mauVat->c)*0.0005f );
      float tiSoNguoc = 1.0f - tiSo;
      ketQua.d = mauVat->d*tiSo + tiSoNguoc*mauTroi->d;
      ketQua.l = mauVat->l*tiSo + tiSoNguoc*mauTroi->l;
      ketQua.x = mauVat->x*tiSo + tiSoNguoc*mauTroi->x;
      ketQua.dd = mauVat->dd*tiSo + tiSoNguoc*mauTroi->dd;
   }
   else {  // không có sương mù, chỉ tô màu vật thể
      ketQua.d = mauVat->d;
      ketQua.l = mauVat->l;
      ketQua.x = mauVat->x;
      ketQua.dd = mauVat->dd;
   }

   return ketQua;
}


#pragma mark ---- Hình Cầu
HinhCau datHinhCau( float banKinh, BaoBi *baoBiVT ) {
   
   HinhCau hinhCau;
   
   hinhCau.banKinh = banKinh;
   
   // ---- tính hợp bao bì
   baoBiVT->gocCucTieu.x = -banKinh;
   baoBiVT->gocCucDai.x = banKinh;
   baoBiVT->gocCucTieu.y = -banKinh;
   baoBiVT->gocCucDai.y = banKinh;
   baoBiVT->gocCucTieu.z = -banKinh;
   baoBiVT->gocCucDai.z = banKinh;
   
   return hinhCau;
}

float xemCatHinhCau( HinhCau *hinhCau, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung ) {
   
   float nghiemGanNhat = kVO_CUC;

   // ---- tính vectơ từ trung tâm hình cầu đến điểm nhìn
   Vecto huongDenHinhCau;
   huongDenHinhCau.x = tia->goc.x;
   huongDenHinhCau.y = tia->goc.y;
   huongDenHinhCau.z = tia->goc.z;

   float A = tia->huong.x*tia->huong.x + tia->huong.y*tia->huong.y + tia->huong.z*tia->huong.z;
   float B = 2.0f*(tia->huong.x*huongDenHinhCau.x + tia->huong.y*huongDenHinhCau.y + tia->huong.z*huongDenHinhCau.z);
   float C = huongDenHinhCau.x*huongDenHinhCau.x + huongDenHinhCau.y*huongDenHinhCau.y + huongDenHinhCau.z*huongDenHinhCau.z -
                 hinhCau->banKinh*hinhCau->banKinh;
   float D = B*B - 4.0f*A*C;


   if( D > 0.0f ) {
     
      // ---- tính nghiệm và nghiệm gần nhất, xài giải thuật chính xác hơn
      float Q;
      if( B < 0.0f )
         Q = -0.5f*(B - sqrtf(D) );
      else
         Q = -0.5f*(B + sqrtf(D) );
   
      float nghiem0 = Q/A;
      float nghiem1 = C/Q;

      // ---- coi trừng nghiệm âm và khác không vỉ tia coi thể bắt đẩu tại mặt của hình cầu
      if( nghiem0 < 0.001f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.001f )
         nghiem1 = kVO_CUC;

      // ---- gởi lại nghiệm nhỏ nhất mà dương
      if( nghiem0 < nghiem1 )
         nghiemGanNhat = nghiem0;
      else
         nghiemGanNhat = nghiem1;
   }

   if( nghiemGanNhat < kVO_CUC ) {
      diemTrung->x = tia->goc.x + nghiemGanNhat*tia->huong.x;
      diemTrung->y = tia->goc.y + nghiemGanNhat*tia->huong.y;
      diemTrung->z = tia->goc.z + nghiemGanNhat*tia->huong.z;
      // ---- vectơ vuông góc cho phát tia tiếp
      phapTuyen->x = diemTrung->x;
      phapTuyen->y = diemTrung->y;
      phapTuyen->z = diemTrung->z;

      donViHoa( phapTuyen );
   }
//   printf( "HinhCau: nghiemGanNhat %5.3f\n", nghiemGanNhat );
   return nghiemGanNhat;
}

unsigned char xemDiemTrongHinhCau( HinhCau *hinhCau, Vecto *diem ) {
   
   if( diem->x*diem->x + diem->y*diem->y + diem->z*diem->z < hinhCau->banKinh*hinhCau->banKinh )
      return kDUNG;
   else
      return kSAI;
}


#pragma mark ---- Mặt Phẳng
MatPhang datMatPhang( Vecto *viTri, float beRong, float beDai, BaoBi *baoBiVT ) {
   MatPhang matPhang;
   
   matPhang.viTri.x = viTri->x;
   matPhang.viTri.y = viTri->y;
   matPhang.viTri.z = viTri->z;
   matPhang.beRong = beRong;
   matPhang.beDai = beDai;

   // ---- tính hợp quanh hình cầu
   matPhang.hopQuanh[0] = -0.5f*beRong;
   matPhang.hopQuanh[1] = 0.5f*beRong;
   matPhang.hopQuanh[2] = 0.0f;
   matPhang.hopQuanh[3] = 0.0f;
   matPhang.hopQuanh[4] = -0.5f*beDai;
   matPhang.hopQuanh[5] = 0.5f*beDai;
   
   return matPhang;
}

float xemCatMatPhang( MatPhang *matPhang, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung ) {
   
   float cachXa = kVO_CUC;
   // ---- xem nếu tia songๆ với mặt phẳng
   if( tia->huong.y == 0.0f )
      return kVO_CUC;
   else {

      // ---- tính cách xa
      cachXa = (matPhang->viTri.y - tia->goc.y)/tia->huong.y;
      if( cachXa < 0.01f )  // điểm cắt ở đang sau tia
         return kVO_CUC;

      // ---- xem hướng x
      float toaDoX = tia->goc.x + cachXa*tia->huong.x;
      if( toaDoX < matPhang->viTri.x - matPhang->beRong*0.5f )
         return kVO_CUC;
      else if( toaDoX > matPhang->viTri.x + matPhang->beRong*0.5f )
         return kVO_CUC;
      
      // ---- xem hướng z
      float toaDoZ = tia->goc.z + cachXa*tia->huong.z;
      if( toaDoZ < matPhang->viTri.z - matPhang->beDai*0.5f )
         return kVO_CUC;
      else if( toaDoZ > matPhang->viTri.z + matPhang->beDai*0.5f )
         return kVO_CUC;
   }
   
   // ---- tíh pháp tuyến
   if( tia->huong.y < 0.0f ) {  // mặt trúng ở trên
      phapTuyen->x = 0.0f;
      phapTuyen->y = 1.0f;
      phapTuyen->z = 0.0f;
   }
   else {     // mặt trúng ở dưới
      phapTuyen->x = 0.0f;
      phapTuyen->y = -1.0f;
      phapTuyen->z = 0.0f;
   }

   diemTrung->x = tia->goc.x + cachXa*tia->huong.x;
   diemTrung->y = tia->goc.y + cachXa*tia->huong.y;
   diemTrung->z = tia->goc.z + cachXa*tia->huong.z;
   return cachXa;
}

#pragma mark --- Hộp
#define kHUONG_AM_X    0
#define kHUONG_DUONG_X 1
#define kHUONG_AM_Y    2
#define kHUONG_DUONG_Y 3
#define kHUONG_AM_Z    4
#define kHUONG_DUONG_Z 5

Hop datHop( float rong, float cao, float dai, BaoBi *baoBiVT ) {

   Hop hop;
   hop.beRong = rong;
   hop.beCao = cao;
   hop.beDai = dai;

   baoBiVT->gocCucTieu.x = -0.5f*rong;
   baoBiVT->gocCucDai.x = 0.5f*rong;
   baoBiVT->gocCucTieu.y = -0.5f*cao;
   baoBiVT->gocCucDai.y = 0.5f*cao;
   baoBiVT->gocCucTieu.z = -0.5f*dai;
   baoBiVT->gocCucDai.z = 0.5f*dai;

   return hop;
}

// ---- Phương pháp của Smits
float xemCatHop( Hop *hop, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung ) {

   unsigned char huongPhapTuyen;
   phapTuyen->x = 0.0f;
   phapTuyen->y = 0.0f;
   phapTuyen->z = 0.0f;

   float nghiemGanNhat = kVO_CUC;  // gần nhất và dương
   float nuaBeRong = 0.5f*hop->beRong;
   float nuaBeCao = 0.5f*hop->beCao;
   float nuaBeDai = 0.5f*hop->beDai;

   // ---- tính nghiệm cho hướng x
   if( tia->huong.x != 0.0f ) {
      // ---- tính hai nghiệm
      float nghiem0 = (-nuaBeRong - tia->goc.x) / tia->huong.x;
      float nghiem1 = (nuaBeRong - tia->goc.x) / tia->huong.x;

      // ----
      if( nghiem0 < 0.0001f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.0001f )
         nghiem1 = kVO_CUC;
      
      float nghiemX;
      unsigned char huongPhapTuyenX;
      if( nghiem0 < nghiem1 ) {
         nghiemX = nghiem0;
         huongPhapTuyenX = kHUONG_AM_X;
      }
      else {
         nghiemX = nghiem1;
         huongPhapTuyenX = kHUONG_DUONG_X;
      }
//      printf("hop: nghiemX %5.3f\n", nghiemX );
      if( nghiemX < nghiemGanNhat ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungX;
         diemTrungX.x = tia->goc.x + nghiemX*tia->huong.x;
         diemTrungX.y = tia->goc.y + nghiemX*tia->huong.y;
         diemTrungX.z = tia->goc.z + nghiemX*tia->huong.z;
//      printf("nghiemX %5.3f  %5.3f\n", nghiemX, nghiemGanNhat );
         if( diemTrungX.y > nuaBeCao )
            nghiemX = kVO_CUC;
         else if( diemTrungX.y < -nuaBeCao )
            nghiemX = kVO_CUC;
         else if( diemTrungX.z > nuaBeDai )
            nghiemX = kVO_CUC;
         else if( diemTrungX.z < -nuaBeDai )
            nghiemX = kVO_CUC;

//      printf("hop: diemTrungX %5.3f %5.3f %5.3f\n", diemTrungX.x, diemTrungX.y, diemTrungX.z );
         if( nghiemX < nghiemGanNhat ) {
            nghiemGanNhat = nghiemX;
            huongPhapTuyen = huongPhapTuyenX;
            diemTrung->x = diemTrungX.x;
            diemTrung->y = diemTrungX.y;
            diemTrung->z = diemTrungX.z;
         }
      }
   }
   
   // ---- tính nghiệm cho hướng y
   if( tia->huong.y != 0.0f ) {
      // ---- tính hai nghiệm
      float nghiem0 = (-nuaBeCao - tia->goc.y) / tia->huong.y;
      float nghiem1 = (nuaBeCao - tia->goc.y) / tia->huong.y;

      // ----
      if( nghiem0 < 0.0001f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.0001f )
         nghiem1 = kVO_CUC;
      
      float nghiemY;
      unsigned char huongPhapTuyenY;
      if( nghiem0 < nghiem1 ) {
         nghiemY = nghiem0;
         huongPhapTuyenY = kHUONG_AM_Y;
      }
      else {
         nghiemY = nghiem1;
         huongPhapTuyenY = kHUONG_DUONG_Y;
      }
//      printf("hop: nghiemY %5.3f\n", nghiemY );
      if( nghiemY < nghiemGanNhat ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungY;
         diemTrungY.x = tia->goc.x + nghiemY*tia->huong.x;
         diemTrungY.y = tia->goc.y + nghiemY*tia->huong.y;
         diemTrungY.z = tia->goc.z + nghiemY*tia->huong.z;
         
         if( diemTrungY.x > nuaBeRong )
            nghiemY = kVO_CUC;
         else if( diemTrungY.x < -nuaBeRong )
            nghiemY = kVO_CUC;
         else if( diemTrungY.z > nuaBeDai )
            nghiemY = kVO_CUC;
         else if( diemTrungY.z < -nuaBeDai )
            nghiemY = kVO_CUC;
         
         if( nghiemY < nghiemGanNhat ) {
            nghiemGanNhat = nghiemY;
            huongPhapTuyen = huongPhapTuyenY;
            diemTrung->x = diemTrungY.x;
            diemTrung->y = diemTrungY.y;
            diemTrung->z = diemTrungY.z;
         }
      }
   }

   // ---- tính nghiệm cho hướng z
   if( tia->huong.z != 0.0f ) {
      // ---- tính hai nghiệm
      float nghiem0 = (-nuaBeDai - tia->goc.z) / tia->huong.z;
      float nghiem1 = (nuaBeDai - tia->goc.z) / tia->huong.z;

      // ----
      if( nghiem0 < 0.0001f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.0001f )
         nghiem1 = kVO_CUC;
      
      float nghiemZ;
      unsigned char huongPhapTuyenZ;
      if( nghiem0 < nghiem1 ) {
         nghiemZ = nghiem0;
         huongPhapTuyenZ = kHUONG_AM_Z;
      }
      else {
         nghiemZ = nghiem1;
         huongPhapTuyenZ = kHUONG_DUONG_Z;
      }
 //     printf("hop: nghiemZ %5.3f\n", nghiemZ );
      if( nghiemZ < nghiemGanNhat ) {
         // ---- xem nếu ở trong mặt
         Vecto diemTrungZ;
         diemTrungZ.x = tia->goc.x + nghiemZ*tia->huong.x;
         diemTrungZ.y = tia->goc.y + nghiemZ*tia->huong.y;
         diemTrungZ.z = tia->goc.z + nghiemZ*tia->huong.z;

         if( diemTrungZ.x > nuaBeRong )
            nghiemZ = kVO_CUC;
         else if( diemTrungZ.x < -nuaBeRong )
            nghiemZ = kVO_CUC;
         else if( diemTrungZ.y > nuaBeCao )
            nghiemZ = kVO_CUC;
         else if( diemTrungZ.y < -nuaBeCao )
            nghiemZ = kVO_CUC;

//      printf("hop: diemTrungZ %5.3f %5.3f %5.3f   nghiemZ %5.3f\n", diemTrungZ.x, diemTrungZ.y, diemTrungZ.z, nghiemZ );
         if( nghiemZ < nghiemGanNhat ) {
            huongPhapTuyen = huongPhapTuyenZ;
            diemTrung->x = diemTrungZ.x;
            diemTrung->y = diemTrungZ.y;
            diemTrung->z = diemTrungZ.z;
            nghiemGanNhat = nghiemZ;
         }
      }
   }

   if( nghiemGanNhat < kVO_CUC ) {
      // ---- tạo pháp tuyến
      if( huongPhapTuyen == kHUONG_AM_X ) {
         phapTuyen->x = -1.0f;
         phapTuyen->y = 0.0f;
         phapTuyen->z = 0.0f;
      }
      else if( huongPhapTuyen == kHUONG_DUONG_X ) {
         phapTuyen->x = 1.0f;
         phapTuyen->y = 0.0f;
         phapTuyen->z = 0.0f;
      }
      else if( huongPhapTuyen == kHUONG_AM_Y ) {
         phapTuyen->x = 0.0;
         phapTuyen->y = -1.0f;
         phapTuyen->z = 0.0f;
      }
      else if( huongPhapTuyen == kHUONG_DUONG_Y ) {
         phapTuyen->x = 0.0;
         phapTuyen->y = 1.0f;
         phapTuyen->z = 0.0f;
      }
      else if( huongPhapTuyen == kHUONG_AM_Z ) {
         phapTuyen->x = 0.0;
         phapTuyen->y = 0.0f;
         phapTuyen->z = -1.0f;
      }
      else if( huongPhapTuyen == kHUONG_DUONG_Z ) {
         phapTuyen->x = 0.0;
         phapTuyen->y = 0.0f;
         phapTuyen->z = 1.0f;
      }
   }
//   printf( "Hop: nghiemGanNhat %5.3f\n", nghiemGanNhat );
   return nghiemGanNhat;
}

unsigned char xemDiemTrongHop( Hop *hop, Vecto *diem ) {
   float nuaBeRong = 0.5f*hop->beRong;
   float nuaBeCao = 0.5f*hop->beCao;
   float nuaBeDai = 0.5f*hop->beDai;
   
   unsigned char diemTrong = kDUNG;
   if( diem->x < -nuaBeRong )
      diemTrong = kSAI;
   else if( diem->x > nuaBeRong )
      diemTrong = kSAI;
   else if( diem->y < -nuaBeCao )
      diemTrong = kSAI;
   else if( diem->y > nuaBeCao )
      diemTrong = kSAI;
   else if( diem->z < -nuaBeDai )
      diemTrong = kSAI;
   else if( diem->z > nuaBeDai )
      diemTrong = kSAI;
   
   return diemTrong;
}

#pragma mark ---- Hình Trụ
HinhTru datHinhTru( float banKinh, float beCao, BaoBi *baoBiVT ) {

   HinhTru hinhTru;
   hinhTru.banKinh = banKinh;
   hinhTru.beCao = beCao;

   baoBiVT->gocCucTieu.x = -banKinh;
   baoBiVT->gocCucDai.x = banKinh;
   baoBiVT->gocCucTieu.y = -0.5*beCao;
   baoBiVT->gocCucDai.y = 0.5f*beCao;
   baoBiVT->gocCucTieu.z = -banKinh;
   baoBiVT->gocCucDai.z = banKinh;
   return hinhTru;
}

// ---- cho biết cắt nắp nào
#define kTRUNG_DUOI 1
#define kTRUNG_TREN 2
float xemCatHinhTru( HinhTru *hinhTru, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung ) {
   
   float nghiemGanNhat = kVO_CUC;

   // ---- tính vectơ từ trung tâm hình cầu đến điểm nhìn
   Vecto huongDenHinhTru;   // hướng đến trung tâm hình trụ
   huongDenHinhTru.x = tia->goc.x;
//   huongDenHinhTru.y = tia->goc.y ; // <---- không xài này
   huongDenHinhTru.z = tia->goc.z;
   
   // ---- tính bán kính bình vì xài nhiều lần
   float banKinhBinh = hinhTru->banKinh*hinhTru->banKinh;
   
   // ---- xem cất hình cao vô cung
   float A = tia->huong.x*tia->huong.x + tia->huong.z*tia->huong.z;
   float B = 2.0f*(tia->huong.x*huongDenHinhTru.x + tia->huong.z*huongDenHinhTru.z);
   float C = huongDenHinhTru.x*huongDenHinhTru.x + huongDenHinhTru.z*huongDenHinhTru.z - banKinhBinh;
   float D = B*B - 4.0f*A*C;

   if( D > 0.0f ) {
      
      // ---- tính nghiệm và nghiệm gần nhất, xài giải thuật chính xác hơn
      float Q;
      if( B < 0.0f )
         Q = -0.5f*(B - sqrtf(D) );
      else
         Q = -0.5f*(B + sqrtf(D) );
      
      float nghiem0 = Q/A;
      float nghiem1 = C/Q;

      // ---- coi trừng nghiệm âm và khác không vỉ tia coi thể bắt đẩu tại mặt của hình cầu
      if( nghiem0 < 0.001f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.001f )
         nghiem1 = kVO_CUC;
      
      // ---- gởi lại nghiệm nhỏ nhất mà dương
      if( nghiem0 < nghiem1 )
         nghiemGanNhat = nghiem0;
      else
         nghiemGanNhat = nghiem1;
   }

   // ---- biết có cắt trụ cao vô cụng nhưng chư biết có cắt trụ ở trên và ở dưới hai mặt của hình trụ
   unsigned char catNap = kSAI;  // cắt nắp
   if( nghiemGanNhat < kVO_CUC ) {

      float yMatTren = 0.5f*hinhTru->beCao;
      float yMatDuoi = -0.5f*hinhTru->beCao;
      // ---- tính trúng mặt tường hình trụ ở đâu
      float toaDoY = tia->goc.y + nghiemGanNhat*tia->huong.y;
      float nghiemTren = (yMatTren - tia->goc.y)/tia->huong.y;
      float nghiemDuoi = (yMatDuoi - tia->goc.y)/tia->huong.y;
      if( nghiemTren < 0.001f )
         nghiemTren = kVO_CUC;
      if( nghiemDuoi < 0.001f )
         nghiemDuoi = kVO_CUC;
      
      // ---- nếu trúng mặt phải trụng cái nào gần hơn
      unsigned char nap;
      float nghiemYGanNhat;
      if( nghiemDuoi < nghiemTren ) {
         nghiemYGanNhat = nghiemDuoi;
         nap = kTRUNG_DUOI;
      }
      else {
         nghiemYGanNhat = nghiemTren;
         nap = kTRUNG_TREN;
      }
      
      Vecto diemTrungGan;
      diemTrungGan.x = tia->goc.x + nghiemYGanNhat*tia->huong.x;
      //    diemTrungTren.y = tia->goc.y + nghiemYGanNhat*tia->huong.y;    không xài
      diemTrungGan.z = tia->goc.z + nghiemYGanNhat*tia->huong.z;
      // ---- tính cách từ tâm trên nắp
      float cachTamTrenNap = diemTrungGan.x*diemTrungGan.x + diemTrungGan.z*diemTrungGan.z;
      
      // ---- cắt tường
      if( toaDoY < yMatTren && toaDoY > yMatDuoi ) {
         // ---- cho tường hợp tia ở trong hình trụ
         if( (nghiemYGanNhat < nghiemGanNhat) && (cachTamTrenNap < banKinhBinh) ) {
            catNap = nap;
            nghiemGanNhat = nghiemYGanNhat;
            diemTrung->x = diemTrungGan.x;
            diemTrung->y = tia->goc.y + nghiemYGanNhat*tia->huong.y;
            diemTrung->z = diemTrungGan.z;
         }
         else
            catNap = kSAI;
      }
      // ---- xem cắt nắp hay tường hay không cắt luôn
      else {

         if( cachTamTrenNap < banKinhBinh ) {
            catNap = nap;
            nghiemGanNhat = nghiemYGanNhat;
            diemTrung->x = diemTrungGan.x;
            diemTrung->y = tia->goc.y + nghiemYGanNhat*tia->huong.y;
            diemTrung->z = diemTrungGan.z;
         }
         else
            nghiemGanNhat = kVO_CUC;

      }
   }
   
   // ---- pháp tuyến cho tường hình trụ
   if( nghiemGanNhat < kVO_CUC && !catNap ) {
      // ---- tính điểm gần nhất
      diemTrung->x = tia->goc.x + nghiemGanNhat*tia->huong.x;
      diemTrung->y = tia->goc.y + nghiemGanNhat*tia->huong.y;
      diemTrung->z = tia->goc.z + nghiemGanNhat*tia->huong.z;
      // ---- vectơ vuông góc cho phát tia tiếp
      phapTuyen->x = diemTrung->x;
      phapTuyen->y = 0.0f;
      phapTuyen->z = diemTrung->z;
      donViHoa( phapTuyen );
   }
   else if( catNap == kTRUNG_TREN ) {  // cắt nắp trên
      phapTuyen->x = 0.0f;
      phapTuyen->y = 1.0f;
      phapTuyen->z = 0.0f;
   }
   else if( catNap == kTRUNG_DUOI ) {  // cắt nắp dưới
      phapTuyen->x = 0.0f;
      phapTuyen->y = -1.0f;
      phapTuyen->z = 0.0f;
   }

   return nghiemGanNhat;
}


unsigned char xemDiemTrongHinhTru( HinhTru *hinhTru, Vecto *diem ) {
   
   unsigned char diemTrong = kDUNG;
   if( diem->x*diem->x + diem->z*diem->z > hinhTru->banKinh*hinhTru->banKinh )
      diemTrong = kSAI;
   
   float nuaBeCao = hinhTru->beCao*0.5f;
   
   if( diemTrong ) {
      if( diem->y < -nuaBeCao )
         diemTrong = kSAI;
      else if( diem->y > nuaBeCao )
         diemTrong = kSAI;
   }
//   printf( "xemTrongHinhTru %d   diem %5.3f %5.3f %5.3f\n", diemTrong, diem->x, diem->y, diem->z );
   return diemTrong;
}

#pragma mark ---- Hình Nón
HinhNon datHinhNon( float banKinhDuoi, float banKinhTren, float beCao, BaoBi *baoBi ) {
   
   HinhNon hinhNon;
   
   hinhNon.banKinhDuoi = banKinhDuoi;
   hinhNon.banKinhTren = banKinhTren;
   hinhNon.beCao = beCao;
   
   // ---- cần bán kín lớn nhất cho bao bì
   float banKinhLonNhat = banKinhDuoi > banKinhTren ? banKinhDuoi : banKinhTren;
   
   baoBi->gocCucTieu.x = -banKinhLonNhat;
   baoBi->gocCucDai.x = banKinhLonNhat;
   baoBi->gocCucTieu.y = -0.5f*beCao;
   baoBi->gocCucDai.y = 0.5f*beCao;
   baoBi->gocCucTieu.z = -banKinhLonNhat;
   baoBi->gocCucDai.z = banKinhLonNhat;
   
   return hinhNon;
}


// bán kính: B(y) = B_1 + (B_2 - B_1)/C (y - y_0 + C/2)
// B_1 bán kính 1   B_2 bán kính 2   C bề cao
// cộng thức: (x - x_0)^2 + (z - z_0)^2 = B(y)^2
// Thử kiếm điểm gần nhất cắt nắp hình nón nếu có, xong
// thử kiếm điểm gần nhất cắt mặt nghiêng hình nón nếu có
// Xong rồi gởi lại điểm nào gần nhất của nắp hay mặt
float xemCatHinhNon( HinhNon *hinhNon, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung ) {
   
   // ==== KIẾM NGHIỆM NẮP
   float nghiemGanNhatNap = kVO_CUC;
   Vecto phapTuyenGanNhatNap;   // pháp tuyến gần nhật cho nắp trên
   
   Vecto phapTuyenNapTren;   // pháp tuyến cho nắp trên
   Vecto phapTuyenNapDuoi;   // pháp tuyến cho nắp dưới
   
   float nuaBeCao = hinhNon->beCao*0.5f;
   
   // ---- nếu có nghiệm nó phải ở trng phạm vị này
   float nghiemNapTren = kVO_CUC;
   float nghiemNapDuoi = kVO_CUC;
   
   if( tia->huong.y != 0.0f ) {  // ---- không thể cắt mặt trên hay dưới nếu tia không có thành phân y (lên/xuống)
      nghiemNapTren = (nuaBeCao - tia->goc.y)/tia->huong.y;
      nghiemNapDuoi = (-nuaBeCao - tia->goc.y)/tia->huong.y;
      
      if( nghiemNapTren > 0.01f ) { // chỉ được cắt nắp trên nếu hướng tia < 0
         diemTrung->x = tia->goc.x + nghiemNapTren*tia->huong.x;
         diemTrung->y = tia->goc.y + nghiemNapTren*tia->huong.y;
         diemTrung->z = tia->goc.z + nghiemNapTren*tia->huong.z;
         //         printf( "xemCatHinhNon: banKinhTrenBinh %5.3f\n", hinhNon->banKinhTren*hinhNon->banKinhTren );
         if( (diemTrung->x*diemTrung->x + diemTrung->z*diemTrung->z) < hinhNon->banKinhTren*hinhNon->banKinhTren ) {
            //            printf( "xemCatHinhNon: trungNapTren %5.3f\n", nghiemNapTren );
            phapTuyenNapTren.x = 0.0f;
            phapTuyenNapTren.y = 1.0f;
            phapTuyenNapTren.z = 0.0f;
         }
         else
            nghiemNapTren = kVO_CUC;
      }
      else
         nghiemNapTren = kVO_CUC;
      
      if( nghiemNapDuoi > 0.01f ) { // chỉ được cắt nắp trên nếu hướng tia < 0
         diemTrung->x = tia->goc.x + nghiemNapDuoi*tia->huong.x;
         diemTrung->y = tia->goc.y + nghiemNapDuoi*tia->huong.y;
         diemTrung->z = tia->goc.z + nghiemNapDuoi*tia->huong.z;
         //        printf( "xemCatHinhNon: banKinhDuoiBinh %5.3f\n", hinhNon->banKinhDuoi*hinhNon->banKinhDuoi );
         if( (diemTrung->x*diemTrung->x + diemTrung->z*diemTrung->z) < hinhNon->banKinhDuoi*hinhNon->banKinhDuoi ) {
            //           printf( "xemCatHinhNon: trungNapDuoi %5.3f\n", nghiemNapDuoi );
            phapTuyenNapDuoi.x = 0.0f;
            phapTuyenNapDuoi.y = -1.0f;
            phapTuyenNapDuoi.z = 0.0f;
         }
         else
            nghiemNapDuoi = kVO_CUC;
      }
      else
         nghiemNapDuoi = kVO_CUC;
      
   }
   // ---- giữ nghệm gần nhất
   if( nghiemNapDuoi < nghiemNapTren ) {
      nghiemGanNhatNap = nghiemNapDuoi;
      phapTuyenGanNhatNap = phapTuyenNapDuoi;
   }
   else {
      nghiemGanNhatNap = nghiemNapTren;
      phapTuyenGanNhatNap = phapTuyenNapTren;
   }
   
 //  printf( "xemCatHinhNon: nghiemGanNhatNap %5.3f nghiemNapTren %5.3f napDuoi %5.3f\n", nghiemGanNhatNap, nghiemNapTren, nghiemNapDuoi );
//   printf( "xemCatHinhNon: phapTuyenGanNhatNap %5.3f %5.3f %5.3f\n", phapTuyenGanNhatNap.x, phapTuyenGanNhatNap.y, phapTuyenGanNhatNap.z );
   
   // ==== KIẾM NGHIỆM MẶT
   float nghiemGanNhatMat = kVO_CUC;
   
   // ---- tính trị số dốc
   float triSoDoc = (hinhNon->banKinhTren - hinhNon->banKinhDuoi)/hinhNon->beCao;
   // ---- tính vectơ từ trung tâm hình cầu đến điểm nhìn
   Vecto huongDenHinhNon;   // hướng đến trung tâm hình trụ
   huongDenHinhNon.x = tia->goc.x;
   huongDenHinhNon.y = (tia->goc.y + nuaBeCao)*triSoDoc + hinhNon->banKinhDuoi;
   huongDenHinhNon.z = tia->goc.z;
   
   float A = tia->huong.x*tia->huong.x - tia->huong.y*tia->huong.y*triSoDoc*triSoDoc + tia->huong.z*tia->huong.z;
   float B = 2.0f*(tia->huong.x*huongDenHinhNon.x - tia->huong.y*triSoDoc*huongDenHinhNon.y + tia->huong.z*huongDenHinhNon.z);
   float C = huongDenHinhNon.x*huongDenHinhNon.x + huongDenHinhNon.z*huongDenHinhNon.z - huongDenHinhNon.y*huongDenHinhNon.y;
   float D = B*B - 4.0f*A*C;
   
   if( D > 0.0f ) {
      // ---- tính nghiệm và nghiệm gần nhất, xài giải thuật chính xác hơn
      float Q;
      if( B < 0.0f )
         Q = -0.5f*(B - sqrtf(D) );
      else
         Q = -0.5f*(B + sqrtf(D) );
      
      float nghiemMat0 = Q/A;
      float nghiemMat1 = C/Q;
//      printf( "xemCatHinhNon: nghiemMat %5.3f  %5.3f\n", nghiemMat0, nghiemMat1 );
      
      // ---- coi trừng nghiệm âm và khác không vỉ tia coi thể bắt đẩu tại mặt của hình cầu
      if( nghiemMat0 < 0.01f )
         nghiemMat0 = kVO_CUC;
      if( nghiemMat1 < 0.01f )
         nghiemMat1 = kVO_CUC;
      
      // ---- xem tia cắt mặt trong phạm vi hình non
      if( nghiemMat0 < kVO_CUC ) {
         float diemTrung0Y = tia->goc.y + nghiemMat0*tia->huong.y;
         
         if( diemTrung0Y > nuaBeCao )
            nghiemMat0 = kVO_CUC;
         else if( diemTrung0Y < -nuaBeCao )
            nghiemMat0 = kVO_CUC;
      }
      
      if( nghiemMat1 < kVO_CUC ) {
         float diemTrung1Y = tia->goc.y + nghiemMat1*tia->huong.y;
         if( diemTrung1Y > nuaBeCao )
            nghiemMat1 = kVO_CUC;
         else if( diemTrung1Y < -nuaBeCao )
            nghiemMat1 = kVO_CUC;
      }
      
      // ---- gởi lại nghiệm nhỏ nhất mà dương
      if( nghiemMat0 < nghiemMat1 ) {
         nghiemGanNhatMat = nghiemMat0;
      }
      else {
         nghiemGanNhatMat = nghiemMat1;

      }
      
   }
//   printf( "xemCatHinhNon: nghiemGanNhatMat %5.3f  nghiemGanNhatNap %5.3f\n", nghiemGanNhatMat, nghiemGanNhatNap );
   
   // ---- kiểm tra điểm gần nhất
   float nghiemGanNhat = kVO_CUC;
   
   if( nghiemGanNhatMat < nghiemGanNhatNap ) {
      nghiemGanNhat = nghiemGanNhatMat;
      // ---- vectơ vuông góc cho phát tia tiếp
      diemTrung->x = tia->goc.x + nghiemGanNhat*tia->huong.x;
      diemTrung->y = tia->goc.y + nghiemGanNhat*tia->huong.y;
      diemTrung->z = tia->goc.z + nghiemGanNhat*tia->huong.z;
      phapTuyen->x = diemTrung->x;
      phapTuyen->y = -(hinhNon->banKinhTren + triSoDoc*(diemTrung->y - nuaBeCao))*triSoDoc;
      phapTuyen->z = diemTrung->z;
      
      donViHoa( phapTuyen );
   }
   else if(nghiemGanNhatNap < nghiemGanNhatMat ) {
      nghiemGanNhat = nghiemGanNhatNap;
      // ---- vectơ vuông góc cho phát tia tiếp
      diemTrung->x = tia->goc.x + nghiemGanNhat*tia->huong.x;
      diemTrung->y = tia->goc.y + nghiemGanNhat*tia->huong.y;
      diemTrung->z = tia->goc.z + nghiemGanNhat*tia->huong.z;
      phapTuyen->x = phapTuyenGanNhatNap.x;
      phapTuyen->y = phapTuyenGanNhatNap.y;
      phapTuyen->z = phapTuyenGanNhatNap.z;
      
      donViHoa( phapTuyen );
   }
   
   
//   printf( "++xemCatHinhNon: phapTuyen %5.3f %5.3f %5.3f\n", phapTuyen->x, phapTuyen->y, phapTuyen->z );
//   printf( "++xemCatHinhNon: nghiemGanNhat %5.3f\n", nghiemGanNhat );
   
   // ---- kết qủa
   return nghiemGanNhat;
}


unsigned char xemDiemTrongHinhNon( HinhNon *hinhNon, Vecto *diem ) {
   
   unsigned char diemTrong = kDUNG;
   // ---- tính trị số dốc
   
   float nuaBeCao = hinhNon->beCao*0.5f;
   float triSoDoc = (hinhNon->banKinhTren - hinhNon->banKinhDuoi)/hinhNon->beCao;
   // ---- bán kín: B(y) = B_1 + (B_2 - B_1)/C (y + C/2)   y tương đối với tâm hình nón
   float banKinh = hinhNon->banKinhDuoi + triSoDoc*(diem->y + nuaBeCao);
   
   if( diem->x*diem->x + diem->z*diem->z > banKinh*banKinh )
      diemTrong = kSAI;
   
   if( diemTrong ) {
      if( diem->y < -nuaBeCao )
         diemTrong = kSAI;
      else if( diem->y > nuaBeCao )
         diemTrong = kSAI;
   }
//   printf( "xemTrongHinhNon %d   diem %5.3f %5.3f %5.3f\n", diemTrong, diem->x, diem->y, diem->z );
   return diemTrong;
}

// hop 6 float [-x, +x, -y, +y, -z, +z]
// phương pháp Smits và có chỉnh sửa bởi Williams, Barrus, Morleey, Shirley cho kiểm tra cắt hợp
/*
unsigned char xemCatHop(const float *hop, Tia *tia ) {

   float tmin, tmax, tymin, tymax, tzmin, tzmax;
   tmin = (hop[tia->dau.x] - tia->goc.x) * tia->daoNghichHuong.x;
   tmax = (hop[1 - tia->dau.x] - tia->goc.x) * tia->daoNghichHuong.x;
   tymin = (hop[tia->dau.y+2] - tia->goc.y) * tia->daoNghichHuong.y;
   tymax = (hop[1-tia->dau.y+2] - tia->goc.y) * tia->daoNghichHuong.y;
   if ( (tmin > tymax) || (tymin > tmax) )
      return kSAI;
   if (tymin > tmin)
      tmin = tymin;
   if (tymax < tmax)
      tmax = tymax;
   tzmin = (hop[tia->dau.z+4] - tia->goc.z) * tia->daoNghichHuong.z;
   tzmax = (hop[1-tia->dau.z+4] - tia->goc.z) * tia->daoNghichHuong.z;
   if ( (tmin > tzmax) || (tzmin > tmax) )
      return kSAI;
   if (tzmin > tmin)
      tmin = tzmin;
   if (tzmax < tmax)
      
      tmax = tzmax;
   return kDUNG; // ( (tmin < t1) && (tmax > t0) );
} */

#pragma mark ---- Mặt Hyberbol
// (x-x_0)^2 + (z-z_0)^2 = R^2 + B*(y-y_0)^2
// B - Biên Độ
MatHyperbol datMatHyperbol( float banKinh, float cachXa, float beCao, BaoBi *baoBiVT ) {
   MatHyperbol matHyperbol;

   matHyperbol.banKinh = banKinh;
   matHyperbol.cachXa = cachXa;
   matHyperbol.beCao = beCao;

   float banKinhCucDai = sqrtf( banKinh + cachXa + 0.25f*beCao*beCao );
   baoBiVT->gocCucTieu.x = -banKinhCucDai;
   baoBiVT->gocCucDai.x = banKinhCucDai;
   baoBiVT->gocCucTieu.y = -0.5f*beCao;
   baoBiVT->gocCucDai.y = 0.5f*beCao;
   baoBiVT->gocCucTieu.z = -banKinhCucDai;
   baoBiVT->gocCucDai.z = banKinhCucDai;

   return matHyperbol;
}

float xemCatMatHyperbol( MatHyperbol *matHyperbol, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung ) {

   float nghiemGanNhat = kVO_CUC;
   
   // ---- tính vectơ từ trung tâm hình cầu đến điểm nhìn
   Vecto huongDenMatHyperbol;   // hướng đến trung tâm hình trụ
   huongDenMatHyperbol.x = tia->goc.x;
   huongDenMatHyperbol.y = tia->goc.y;
   huongDenMatHyperbol.z = tia->goc.z;
   
   float A = tia->huong.x*tia->huong.x - tia->huong.y*tia->huong.y*matHyperbol->cachXa + tia->huong.z*tia->huong.z;
   float B = 2.0f*(tia->huong.x*huongDenMatHyperbol.x - tia->huong.y*huongDenMatHyperbol.y*matHyperbol->cachXa + tia->huong.z*huongDenMatHyperbol.z);
   float C = huongDenMatHyperbol.x*huongDenMatHyperbol.x + huongDenMatHyperbol.z*huongDenMatHyperbol.z - huongDenMatHyperbol.y*huongDenMatHyperbol.y*matHyperbol->cachXa
                + matHyperbol->banKinh;
   float D = B*B - 4.0f*A*C;
   
   if( D > 0.0f ) {
      // ---- tính nghiệm và nghiệm gần nhất, xài giải thuật chính xác hơn
      float Q;
      if( B < 0.0f )
         Q = -0.5f*(B - sqrtf(D) );
      else
         Q = -0.5f*(B + sqrtf(D) );
      
      float nghiem0 = Q/A;
      float nghiem1 = C/Q;
      
      // ---- coi trừng nghiệm âm và khác không vỉ tia coi thể bắt đẩu tại mặt của hình cầu
      if( nghiem0 < 0.01f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.01f )
         nghiem1 = kVO_CUC;
      
      // ---- gởi lại nghiệm nhỏ nhất mà dương
      if( nghiem0 < nghiem1 )
         nghiemGanNhat = nghiem0;
      else
         nghiemGanNhat = nghiem1;
   }

   // ---- biết có cắt trụ cao vô cụng nhưng chư biết có cắt trụ ở trên và ở dưới hai mặt của hình trụ
   unsigned char catNap = kSAI;  // cắt nắp
   if( nghiemGanNhat < kVO_CUC ) {
      // ---- kiểm tra tọa độ y của điểm trúng
      float toaDoY = tia->goc.y + nghiemGanNhat*tia->huong.y;
      float yMatTren = matHyperbol->beCao*0.5f;
      float yMatDuoi = -matHyperbol->beCao*0.5f;
      
      if( (toaDoY > yMatTren ) && (tia->huong.y < 0.0f) ) {   // tọa độ y cao hơn mà tia đang bay xuống
         float nghiemY = (yMatTren - tia->goc.y)/tia->huong.y;
         Vecto diemTrungTuongDoi;
         diemTrungTuongDoi.x = tia->goc.x + nghiemY*tia->huong.x;
         diemTrungTuongDoi.y = tia->goc.y + nghiemY*tia->huong.y;
         diemTrungTuongDoi.z = tia->goc.z + nghiemY*tia->huong.z;
         // ---- tính bán kính cắt ngang
         if( diemTrungTuongDoi.x*diemTrungTuongDoi.x + diemTrungTuongDoi.z*diemTrungTuongDoi.z < matHyperbol->banKinh
            + matHyperbol->cachXa + diemTrungTuongDoi.y*diemTrungTuongDoi.y ) {
            catNap = kTRUNG_TREN;
            nghiemGanNhat = nghiemY;
            diemTrung->x = tia->goc.x + nghiemY*tia->huong.x;
            diemTrung->y = tia->goc.y + nghiemY*tia->huong.y;
            diemTrung->z = tia->goc.z + nghiemY*tia->huong.z;
         }
         else
            nghiemGanNhat = kVO_CUC;
      }
      else if( (toaDoY < yMatDuoi) && (tia->huong.y > 0.0f) ) { // tọa độ y thấp hơn mà tia đang bay lên
         float nghiemY = (yMatDuoi - tia->goc.y)/tia->huong.y;
         Vecto diemTrungTuongDoi;
         diemTrungTuongDoi.x = tia->goc.x + nghiemY*tia->huong.x;
         diemTrungTuongDoi.y = tia->goc.y + nghiemY*tia->huong.y;
         diemTrungTuongDoi.z = tia->goc.z + nghiemY*tia->huong.z;
         if( diemTrungTuongDoi.x*diemTrungTuongDoi.x + diemTrungTuongDoi.z*diemTrungTuongDoi.z < matHyperbol->banKinh
            + matHyperbol->cachXa + diemTrungTuongDoi.y*diemTrungTuongDoi.y ) {
            catNap = kTRUNG_DUOI;
            nghiemGanNhat = nghiemY;
            diemTrung->x = tia->goc.x + nghiemY*tia->huong.x;
            diemTrung->y = tia->goc.y + nghiemY*tia->huong.y;
            diemTrung->z = tia->goc.z + nghiemY*tia->huong.z;
         }
         else
            nghiemGanNhat = kVO_CUC;
      }
      else if( (toaDoY < yMatTren) && (toaDoY > yMatDuoi) )
         catNap = kSAI;
      else
         nghiemGanNhat = kVO_CUC;    // không cắt hình trụ
   }

   // ---- pháp tuyến cho tường hình trụ
   if( nghiemGanNhat < kVO_CUC && !catNap ) {
      diemTrung->x = tia->goc.x + nghiemGanNhat*tia->huong.x;
      diemTrung->y = tia->goc.y + nghiemGanNhat*tia->huong.y;
      diemTrung->z = tia->goc.z + nghiemGanNhat*tia->huong.z;
      // ---- vectơ vuông góc cho phát tia tiếp
      phapTuyen->x = diemTrung->x;
      phapTuyen->y = -diemTrung->y*matHyperbol->cachXa;
      phapTuyen->z = diemTrung->z;
      donViHoa( phapTuyen );
   }
   else if( catNap == kTRUNG_TREN ) {  // cắt nắp trên
      phapTuyen->x = 0.0f;
      phapTuyen->y = 1.0f;
      phapTuyen->z = 0.0f;
   }
   else if( catNap == kTRUNG_DUOI ) {  // cắt nắp dưới
      phapTuyen->x = 0.0f;
      phapTuyen->y = -1.0f;
      phapTuyen->z = 0.0f;
   }

   return nghiemGanNhat;
}

#pragma mark ---- Mặt Parabol
// xài bán kính dương cho ở trên, âm cho ở dưới 
MatParabol datMatParabol( float banKinh, float beCao, BaoBi *baoBiVT ) {

   MatParabol matParabol;
   matParabol.banKinh = banKinh;
   matParabol.beCao = beCao;
   
   // ---- tinh hộp bao bì
   banKinh *= beCao;

   if( banKinh > 0.0f ) {
      baoBiVT->gocCucTieu.x = -banKinh;
      baoBiVT->gocCucTieu.y = 0.0f;
      baoBiVT->gocCucTieu.z = -banKinh;
      
      baoBiVT->gocCucDai.x = banKinh;
      baoBiVT->gocCucDai.y = beCao;
      baoBiVT->gocCucDai.z = banKinh;
   }
   else {
      baoBiVT->gocCucTieu.x = banKinh;
      baoBiVT->gocCucTieu.y = -beCao;
      baoBiVT->gocCucTieu.z = banKinh;
   
      baoBiVT->gocCucDai.x = -banKinh;
      baoBiVT->gocCucDai.y = 0.0f;
      baoBiVT->gocCucDai.z = -banKinh;
   }

   return matParabol;
}

// cộng thức: (x - x_0)^2 + (z - z_0)^2 = B(y - y0)
float xemCatMatParabol( MatParabol *matParabol, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung ) {
   
   float nghiemGanNhat = kVO_CUC;
   
   // ---- tính vectơ từ trung tâm hình cầu đến điểm nhìn
   Vecto huongDenMatParabol;   // hướng đến trung tâm hình trụ
   huongDenMatParabol.x = tia->goc.x;
   huongDenMatParabol.y = tia->goc.y;
   huongDenMatParabol.z = tia->goc.z;
   
   float A = tia->huong.x*tia->huong.x + tia->huong.z*tia->huong.z;
   float B = 2.0f*(tia->huong.x*huongDenMatParabol.x + tia->huong.z*huongDenMatParabol.z) - tia->huong.y*matParabol->banKinh;
   float C = huongDenMatParabol.x*huongDenMatParabol.x + huongDenMatParabol.z*huongDenMatParabol.z - huongDenMatParabol.y*matParabol->banKinh;
   float D = B*B - 4.0f*A*C;
   
   if( D > 0.0f ) {
      // ---- tính nghiệm và nghiệm gần nhất, xài giải thuật chính xác hơn
      float Q;
      if( B < 0.0f )
         Q = -0.5f*(B - sqrtf(D) );
      else
         Q = -0.5f*(B + sqrtf(D) );
      
      float nghiem0 = Q/A;
      float nghiem1 = C/Q;
      
      // ---- coi trừng nghiệm âm và khác không vỉ tia coi thể bắt đẩu tại mặt của hình cầu
      if( nghiem0 < 0.01f )
         nghiem0 = kVO_CUC;
      if( nghiem1 < 0.01f )
         nghiem1 = kVO_CUC;
      
      // ---- gởi lại nghiệm nhỏ nhất mà dương
      if( nghiem0 < nghiem1 )
         nghiemGanNhat = nghiem0;
      else
         nghiemGanNhat = nghiem1;
   }
   
   // ---- biết có cắt trụ cao vô cụng nhưng chư biết có cắt trụ ở trên và ở dưới hai mặt của hình trụ
   unsigned char catNap = kSAI;  // cắt nắp
   if( nghiemGanNhat < kVO_CUC ) {
      // ---- kiểm tra tọa độ y của điểm trúng
      float toaDoY = tia->goc.y + nghiemGanNhat*tia->huong.y;
      // ---- không giống các mặt khác vì chỉ có mặt một bên
      float yMatTren = matParabol->beCao;
      float yMatDuoi = -matParabol->beCao;
      
      if( (toaDoY > yMatTren ) && (tia->huong.y < 0.0f) ) {   // tọa độ y cao hơn mà tia đang bay xuống
         float nghiemY = (yMatTren - tia->goc.y)/tia->huong.y;
         Vecto diemTrungTuongDoi;
         diemTrungTuongDoi.x = tia->goc.x + nghiemY*tia->huong.x;
         diemTrungTuongDoi.y = tia->goc.y + nghiemY*tia->huong.y;
         diemTrungTuongDoi.z = tia->goc.z + nghiemY*tia->huong.z;
         // ---- tính bán kính cắt ngang
         if( diemTrungTuongDoi.x*diemTrungTuongDoi.x + diemTrungTuongDoi.z*diemTrungTuongDoi.z < matParabol->banKinh*diemTrungTuongDoi.y ) {
            catNap = kTRUNG_TREN;
            nghiemGanNhat = nghiemY;
            diemTrung->x = tia->goc.x + nghiemY*tia->huong.x;
            diemTrung->y = tia->goc.y + nghiemY*tia->huong.y;
            diemTrung->z = tia->goc.z + nghiemY*tia->huong.z;
         }
         else
            nghiemGanNhat = kVO_CUC;
      }
      else if( (toaDoY < yMatDuoi) && (tia->huong.y > 0.0f) ) { // tọa độ y thấp hơn mà tia đang bay lên
         float nghiemY = (yMatDuoi - tia->goc.y)/tia->huong.y;
         Vecto diemTrungTuongDoi;
         diemTrungTuongDoi.x = tia->goc.x + nghiemY*tia->huong.x;
         diemTrungTuongDoi.y = tia->goc.y + nghiemY*tia->huong.y;
         diemTrungTuongDoi.z = tia->goc.z + nghiemY*tia->huong.z;
         if( diemTrungTuongDoi.x*diemTrungTuongDoi.x + diemTrungTuongDoi.z*diemTrungTuongDoi.z < matParabol->banKinh*diemTrungTuongDoi.y ) {
            catNap = kTRUNG_DUOI;
            nghiemGanNhat = nghiemY;
            diemTrung->x = tia->goc.x + nghiemY*tia->huong.x;
            diemTrung->y = tia->goc.y + nghiemY*tia->huong.y;
            diemTrung->z = tia->goc.z + nghiemY*tia->huong.z;
         }
         else
            nghiemGanNhat = kVO_CUC;
      }
      else if( (toaDoY < yMatTren) && (toaDoY > yMatDuoi) )
         catNap = kSAI;
      else
         nghiemGanNhat = kVO_CUC;    // không cắt hình trụ
   }
   
   // ---- pháp tuyến cho tường hình trụ
   if( nghiemGanNhat < kVO_CUC && !catNap ) {
      diemTrung->x = tia->goc.x + nghiemGanNhat*tia->huong.x;
      diemTrung->y = tia->goc.y + nghiemGanNhat*tia->huong.y;
      diemTrung->z = tia->goc.z + nghiemGanNhat*tia->huong.z;
      // ---- vectơ vuông góc cho phát tia tiếp
      phapTuyen->x = diemTrung->x;
      phapTuyen->y = -0.5f*matParabol->banKinh;
      phapTuyen->z = diemTrung->z;
      donViHoa( phapTuyen );
   }
   else if( catNap == kTRUNG_TREN ) {  // cắt nắp trên
      phapTuyen->x = 0.0f;
      phapTuyen->y = 1.0f;
      phapTuyen->z = 0.0f;
   }
   else if( catNap == kTRUNG_DUOI ) {  // cắt nắp dưới
      phapTuyen->x = 0.0f;
      phapTuyen->y = -1.0f;
      phapTuyen->z = 0.0f;
   }
   
   return nghiemGanNhat;
}

#pragma mark ---- Mặt Sóng
MatSong datMatSong( float beRong, float beDai, float bienDo0, float bienDo1, float bienDo2, BaoBi *baoBiVT ) {
   MatSong matSong;
   matSong.bienDo0 = bienDo0;
   matSong.bienDo1 = bienDo1;
   matSong.bienDo2 = bienDo2;
   matSong.bienDoNhat = bienDo0 + bienDo1 + bienDo2;

   baoBiVT->gocCucTieu.x = -0.5f*beRong;
   baoBiVT->gocCucDai.x = 0.5f*beRong;
   baoBiVT->gocCucTieu.y = -matSong.bienDoNhat;
   baoBiVT->gocCucDai.y = matSong.bienDoNhat;
   baoBiVT->gocCucTieu.z = -0.5f*beDai;
   baoBiVT->gocCucDai.z = 0.5f*beDai;

   return matSong;
}

// y = a*sin(A_0*x + B_0*z + C_0*t) + b*sin(A_1*x + B_1*z + C_1*t) + c*sin(A_2*x + B_2*z + C_2*t)
// -a*cos(A_0*x + B_0*z + C_0*t)*A_0 + b*cos(A_1*x + B_1*z + C_1*t)*A_1 + c*cos(A_2*x + B_2*z + C_2*t)*A_2,
//  1,
// -a*cos(A_0*x + B_0*z + C_0*t)*B_0 + b*cos(A_1*x + B_1*z + C_1*t)*B_1 + c*cos(A_2*x + B_2*z + C_2*t)*B_2
#define kA_0  1.0f
#define kB_0  -0.3f
#define kC_0  0.8f

#define kA_1  0.5f
#define kB_1  0.3f
#define kC_1  0.5f
float xemCatMatSong( MatSong *matSong, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung, float thoiGian ) {
   
   float cachXa = kVO_CUC;

   float cachTren = 0.5f;
   float cachDuoi = 0.01f;
   
   float xDuoi = tia->goc.x + cachDuoi*tia->huong.x;
   float yDuoi = tia->goc.y + cachDuoi*tia->huong.y;
   float zDuoi = tia->goc.z + cachDuoi*tia->huong.z;
   float ketQuaDuoi = matSong->bienDo0*sinf( kA_0*xDuoi + kB_0*zDuoi - kC_0*thoiGian ) +
   matSong->bienDo1*sinf( kA_1*xDuoi + kB_1*zDuoi - kC_1*thoiGian )
   - yDuoi;

   unsigned char trenAm = kSAI;
   unsigned char duoiAm = kSAI;
   if( ketQuaDuoi < 0.0f )
      duoiAm = kDUNG;
   else
      duoiAm = kSAI;
   
   // ---- kiếm điểm trúng
   unsigned char xong = kSAI;
   while( !xong ) {
      // ---- tính giá trị của hàm số
      float xTren = tia->goc.x + cachTren*tia->huong.x;
      float yTren = tia->goc.y + cachTren*tia->huong.y;
      float zTren = tia->goc.z + cachTren*tia->huong.z;
      
      // ---- tính gía trị hàm số
      float ketQuaTren = matSong->bienDo0*sinf( kA_0*xTren + kB_0*zTren - kC_0*thoiGian ) +
                  matSong->bienDo1*sinf( kA_1*xTren + kB_1*zTren - kC_1*thoiGian ) - yTren;

      // ---- âm hay dương
      if( ketQuaTren < 0.0f )
         trenAm = kDUNG;
      else
         trenAm = kSAI;
      
      if( trenAm != duoiAm )
         xong = kDUNG;
      else {
         cachTren += 2.0f;
         
      }
      //         printf( "cachDuoi %5.3f  cachTren %5.3f  ketQuaDuoi %5.3f  ketQuaTren %5.3f\n", cachDuoi, cachTren, ketQuaDuoi, ketQuaTren );
      if( cachTren > 400.0f )
         xong = kDUNG;
   }
   
   if( trenAm != duoiAm ) {
      unsigned char soLuongTinh = 0;
      xong = kSAI;
      while( !xong ) {
         // ---- trung điểm
         float cachTrung = (cachTren + cachDuoi)*0.5f;
         float xTrung = tia->goc.x + cachTrung*tia->huong.x;
         float yTrung = tia->goc.y + cachTrung*tia->huong.y;
         float zTrung = tia->goc.z + cachTrung*tia->huong.z;
         float ketQuaTrung = matSong->bienDo0*sinf( kA_0*xTrung + kB_0*zTrung - kC_0*thoiGian ) +
         matSong->bienDo1*sinf( kA_1*xTrung + kB_1*zTrung - kC_1*thoiGian )
         - yTrung;
         
         unsigned char trungAm = kSAI;
         if( ketQuaTrung < 0.0f )
            trungAm = kDUNG;
         
         if( trungAm == trenAm )
            cachTren = cachTrung;
         if( trungAm == duoiAm )
            cachDuoi = cachTrung;
         
         float cach = cachTren - cachDuoi;
         if( cach < 0.0f )
            cach = -cach;
         
         if( cach < 0.001f ) {
            xong = kDUNG;
            cachXa = (cachTren + cachDuoi)*0.5f;
            diemTrung->x = tia->goc.x + cachXa*tia->huong.x;
            diemTrung->y = tia->goc.y + cachXa*tia->huong.y;
            diemTrung->z = tia->goc.z + cachXa*tia->huong.z;
            phapTuyen->x = -matSong->bienDo0*kA_0*cosf( kA_0*diemTrung->x + kB_0*diemTrung->z - kC_0*thoiGian )
            -matSong->bienDo1*kA_1*cosf( kA_1*diemTrung->x + kB_1*diemTrung->z - kC_1*thoiGian );
            phapTuyen->y = 1.0f;
            phapTuyen->z = -matSong->bienDo0*kB_0*cosf( kA_0*diemTrung->x + kB_0*diemTrung->z - kC_0*thoiGian )
            -matSong->bienDo1*kB_1*cosf( kA_1*diemTrung->x + kB_1*diemTrung->z - kC_1*thoiGian );
            donViHoa( phapTuyen );
         }
         
         // ---- đừng tính quá lâu
         if( soLuongTinh > 50 )
            xong = kDUNG;
         
         soLuongTinh++;
      }
   }

//   printf( "cachXa %5.3f\n", cachXa );
   return cachXa;
}

#pragma mark ---- Hình Xuyến
HinhXuyen datHinhXuyen( float banKinhVong, float banKinhOng, BaoBi *baoBiVT ) {
   HinhXuyen hinhXuyen;
   hinhXuyen.banKinhVong = banKinhVong;
   hinhXuyen.banKinhOng = banKinhOng;
   
   baoBiVT->gocCucTieu.x = -banKinhVong - banKinhOng;
   baoBiVT->gocCucDai.x = banKinhVong + banKinhOng;
   baoBiVT->gocCucTieu.y = -banKinhOng;
   baoBiVT->gocCucDai.y = banKinhOng;
   baoBiVT->gocCucTieu.z = baoBiVT->gocCucTieu.x;
   baoBiVT->gocCucDai.z = baoBiVT->gocCucDai.x;
   
   return hinhXuyen;
}

float xemCatHinhXuyen( HinhXuyen *hinhXuyen, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung, float thoiGian ) {
   
   float cachXa = kVO_CUC;
   
   // <----------- chưa làm
   //   printf( "cachXa %5.3f\n", cachXa );
   return cachXa;
}


#pragma mark ---- Tứ Diện
TuDien datTuDien( float beRong, float beCao, float beDai, BaoBi *baoBiVT ) {
   
   TuDien tuDien;
   tuDien.soLuongTamGiac = 4;

   // ---- tính các đỉnh
   tuDien.mangDinh[0].x = 0.0f;
   tuDien.mangDinh[0].y = beCao;
   tuDien.mangDinh[0].z = 0.0f;

   tuDien.mangDinh[1].x = 0.0f;
   tuDien.mangDinh[1].y = -0.5f*beCao;
   tuDien.mangDinh[1].z = 0.868f*beDai;
   
   tuDien.mangDinh[2].x = 0.868f*beRong;
   tuDien.mangDinh[2].y = -0.5f*beCao;
   tuDien.mangDinh[2].z = -0.868f*0.5f*beDai;

   tuDien.mangDinh[3].x = -0.868f*beRong;
   tuDien.mangDinh[3].y = -0.5f*beCao;
   tuDien.mangDinh[3].z = -0.868f*0.5f*beDai;

   // ----
   tuDien.mangTamGiac[0].dinh0 = 0;
   tuDien.mangTamGiac[0].dinh1 = 1;
   tuDien.mangTamGiac[0].dinh2 = 2;

   tuDien.mangTamGiac[1].dinh0 = 0;
   tuDien.mangTamGiac[1].dinh1 = 2;
   tuDien.mangTamGiac[1].dinh2 = 3;

   tuDien.mangTamGiac[2].dinh0 = 0;
   tuDien.mangTamGiac[2].dinh1 = 3;
   tuDien.mangTamGiac[2].dinh2 = 1;
   // ---- dái
   tuDien.mangTamGiac[3].dinh0 = 3;
   tuDien.mangTamGiac[3].dinh1 = 2;
   tuDien.mangTamGiac[3].dinh2 = 1;

   baoBiVT->gocCucTieu.x = -0.868f*beRong;
   baoBiVT->gocCucDai.x = 0.868f*beRong;
   baoBiVT->gocCucTieu.y = -0.5f*beCao;
   baoBiVT->gocCucDai.y = beCao;
   baoBiVT->gocCucTieu.z = -0.5f*0.868f*beDai;
   baoBiVT->gocCucDai.z = 0.868f*beDai;

   return tuDien;
}

float xemCatVatTheTamGiac( Vecto *mangDinh, TamGiac *mangTamGiac, unsigned short soLuongTamGiac, Tia *tia, Vecto *phapTuyen, Vecto *diemTrung ) {
   
   float nghiemGanNhat = kVO_CUC;
//   printf( "Tia %5.3f %5.3f %5.3f  %5.3f %5.3f %5.3f\n", tia->goc.x, tia->goc.y, tia->goc.z, tia->huong.x, tia->huong.y, tia->huong.z );
   
   unsigned char cat = kSAI;
   unsigned char soMat = 0;
   while ( soMat < soLuongTamGiac ) {
      // ---- tám giac 0
      Vecto tamGiac[3];
      tamGiac[0] = mangDinh[mangTamGiac[soMat].dinh0];
      tamGiac[1] = mangDinh[mangTamGiac[soMat].dinh1];
      tamGiac[2] = mangDinh[mangTamGiac[soMat].dinh2];

      Vecto ketQua = tinhPhapTuyenChoTamGiac( tamGiac );
//      printf( "%d/%d  dinh %d %d %d\n", soMat, soLuongTamGiac, mangTamGiac[soMat].dinh0, mangTamGiac[soMat].dinh1, mangTamGiac[soMat].dinh2 );
//      printf( "    PT %5.3f %5.3f %5.3f\n", ketQua.x, ketQua.y, ketQua.z );
      
      float nghiem = xemCatTamGiacMT( tamGiac, tia );
      
      if( nghiem < nghiemGanNhat ) {  // mặt 0
         nghiemGanNhat = nghiem;
         Vecto phapTuyenTamGiac = tinhPhapTuyenChoTamGiac( tamGiac );
         phapTuyen->x = phapTuyenTamGiac.x;
         phapTuyen->y = phapTuyenTamGiac.y;
         phapTuyen->z = phapTuyenTamGiac.z;
         diemTrung->x = tia->goc.x + nghiem*tia->huong.x;
         diemTrung->y = tia->goc.y + nghiem*tia->huong.y;
         diemTrung->z = tia->goc.z + nghiem*tia->huong.z;
      }
      soMat++;
   }
   
   return nghiemGanNhat;
}

#pragma mark ---- Bát Diện
BatDien datBatDien( float beRong, float beCao, float beDai, BaoBi *baoBi ) {
   
   BatDien batDien;
   batDien.soLuongTamGiac = 8;
   float nuaBeRong = 0.5f*beRong;
   float nuaBeDai = 0.5f*beDai;
   
   // ---- cực trên
   batDien.mangDinh[0].x = 0.0;
   batDien.mangDinh[0].y = 0.5f*beCao;
   batDien.mangDinh[0].z = 0.0;
   // ---- xích đạo
   batDien.mangDinh[1].x = nuaBeRong;
   batDien.mangDinh[1].y = 0.0;
   batDien.mangDinh[1].z = nuaBeDai;
   
   batDien.mangDinh[2].x = nuaBeRong;
   batDien.mangDinh[2].y = 0.0f;
   batDien.mangDinh[2].z = -nuaBeDai;

   batDien.mangDinh[3].x = -nuaBeRong;
   batDien.mangDinh[3].y = 0.0f;
   batDien.mangDinh[3].z = -nuaBeDai;

   batDien.mangDinh[4].x = -nuaBeRong;
   batDien.mangDinh[4].y = 0.0f;
   batDien.mangDinh[4].z = nuaBeDai;
   // ---- cực dưới
   batDien.mangDinh[5].x = 0.0;
   batDien.mangDinh[5].y = -0.5f*beCao;
   batDien.mangDinh[5].z = 0.0f;
   
   // ---- mảng tam giác
   // 4 mặt trên
   batDien.mangTamGiac[0].dinh0 = 0;
   batDien.mangTamGiac[0].dinh1 = 1;
   batDien.mangTamGiac[0].dinh2 = 2;
   
   batDien.mangTamGiac[1].dinh0 = 0;
   batDien.mangTamGiac[1].dinh1 = 2;
   batDien.mangTamGiac[1].dinh2 = 3;
   
   batDien.mangTamGiac[2].dinh0 = 0;
   batDien.mangTamGiac[2].dinh1 = 3;
   batDien.mangTamGiac[2].dinh2 = 4;

   batDien.mangTamGiac[3].dinh0 = 0;
   batDien.mangTamGiac[3].dinh1 = 4;
   batDien.mangTamGiac[3].dinh2 = 1;
   // 4 mặt dưới
   batDien.mangTamGiac[4].dinh0 = 2;
   batDien.mangTamGiac[4].dinh1 = 1;
   batDien.mangTamGiac[4].dinh2 = 5;

   batDien.mangTamGiac[5].dinh0 = 3;
   batDien.mangTamGiac[5].dinh1 = 2;
   batDien.mangTamGiac[5].dinh2 = 5;

   batDien.mangTamGiac[6].dinh0 = 4;
   batDien.mangTamGiac[6].dinh1 = 3;
   batDien.mangTamGiac[6].dinh2 = 5;

   batDien.mangTamGiac[7].dinh0 = 1;
   batDien.mangTamGiac[7].dinh1 = 4;
   batDien.mangTamGiac[7].dinh2 = 5;

   baoBi->gocCucTieu.x = -nuaBeRong;
   baoBi->gocCucDai.x = nuaBeRong;
   baoBi->gocCucTieu.y = -0.5f*beCao;
   baoBi->gocCucDai.y = 0.5f*beCao;
   baoBi->gocCucTieu.z = -nuaBeDai;
   baoBi->gocCucDai.z = nuaBeDai;

   return batDien;
}


#pragma mark ---- Thập Nhị Diện
ThapNhiDien datThapNhiDien( float beRong, float beCao, float beDai, BaoBi *baoBi ) {
   
   ThapNhiDien thapNhiDien;
   thapNhiDien.soLuongTamGiac = 60;  // 12 ngũ giảc x 5 tam giác
   
   // ==== các tâm
   // ---- tâm mặt trên
   thapNhiDien.mangDinh[0].x = 0.0f;
   thapNhiDien.mangDinh[0].y = 0.79465f*beCao;
   thapNhiDien.mangDinh[0].z = 0.0f;
   // ---- tâm 5 ngũ giác lớp trên
   thapNhiDien.mangDinh[1].x = 0.417777f*beRong;
   thapNhiDien.mangDinh[1].y = 0.35568f*beCao;
   thapNhiDien.mangDinh[1].z = 0.57551f*beDai;
   
   thapNhiDien.mangDinh[2].x = 0.67597f*beRong;
   thapNhiDien.mangDinh[2].y = 0.35568f*beCao;
   thapNhiDien.mangDinh[2].z = -0.21915f*beDai;

   thapNhiDien.mangDinh[3].x = 0.000000f*beRong;
   thapNhiDien.mangDinh[3].y = 0.35568f*beCao;
   thapNhiDien.mangDinh[3].z = -0.71027f*beDai;

   thapNhiDien.mangDinh[4].x = -0.67597f*beRong;
   thapNhiDien.mangDinh[4].y = 0.35568f*beCao;
   thapNhiDien.mangDinh[4].z = -0.21915f*beDai;

   thapNhiDien.mangDinh[5].x = -0.41777f*beRong;
   thapNhiDien.mangDinh[5].y = 0.35568f*beCao;
   thapNhiDien.mangDinh[5].z = 0.57551f*beDai;
   // ---- tâm 5 ngũ giác lớp dưới
   thapNhiDien.mangDinh[6].x = 0.00000f*beRong;
   thapNhiDien.mangDinh[6].y = -0.35568f*beCao;
   thapNhiDien.mangDinh[6].z = 0.71027f*beDai;
   
   thapNhiDien.mangDinh[7].x = 0.67597f*beRong;
   thapNhiDien.mangDinh[7].y = -0.35568f*beCao;
   thapNhiDien.mangDinh[7].z = 0.21915f*beDai;

   thapNhiDien.mangDinh[8].x = 0.41777f*beRong;
   thapNhiDien.mangDinh[8].y = -0.35568f*beCao;
   thapNhiDien.mangDinh[8].z = -0.57551f*beDai;
   
   thapNhiDien.mangDinh[9].x = -0.41777f*beRong;
   thapNhiDien.mangDinh[9].y = -0.35568f*beCao;
   thapNhiDien.mangDinh[9].z = -0.57551f*beDai;

   thapNhiDien.mangDinh[10].x = -0.67597f*beRong;
   thapNhiDien.mangDinh[10].y = -0.35568f*beCao;
   thapNhiDien.mangDinh[10].z = 0.21915f*beDai;
   // ---- tâm mặt dưới
   thapNhiDien.mangDinh[11].x = 0.00000f*beRong;
   thapNhiDien.mangDinh[11].y = -0.79465f*beCao;
   thapNhiDien.mangDinh[11].z = 0.000000f*beDai;

   // ---- rìa mặt trên
   thapNhiDien.mangDinh[12].x = 0.0f;
   thapNhiDien.mangDinh[12].y = 0.79465451f*beCao;
   thapNhiDien.mangDinh[12].z = 0.60706196f*beRong;

   thapNhiDien.mangDinh[13].x = 0.57735022f*beDai;
   thapNhiDien.mangDinh[13].y = 0.79465451f*beCao;
   thapNhiDien.mangDinh[13].z = 0.18759247f*beRong;
   
   thapNhiDien.mangDinh[14].x = 0.35682211f*beDai;
   thapNhiDien.mangDinh[14].y = 0.79465451f*beCao;
   thapNhiDien.mangDinh[14].z = -0.49112344f*beRong;
   
   thapNhiDien.mangDinh[15].x = -0.35682211f*beDai;
   thapNhiDien.mangDinh[15].y = 0.79465451f*beCao;
   thapNhiDien.mangDinh[15].z = -0.49112344f*beRong;

   thapNhiDien.mangDinh[16].x = -0.57735022f*beDai;
   thapNhiDien.mangDinh[16].y = 0.79465451f*beCao;
   thapNhiDien.mangDinh[16].z = 0.18759247f*beRong;

   // ---- xích đạo
   thapNhiDien.mangDinh[17].x = 0.0f;
   thapNhiDien.mangDinh[17].y = 0.18759248f*beCao;
   thapNhiDien.mangDinh[17].z = 0.98224695f*beRong;
   
   thapNhiDien.mangDinh[18].x = 0.93417235f*beDai;
   thapNhiDien.mangDinh[18].y = 0.18759248f*beCao;
   thapNhiDien.mangDinh[18].z = 0.30353102f*beRong;

   thapNhiDien.mangDinh[19].x = 0.57735022f*beDai;
   thapNhiDien.mangDinh[19].y = 0.18759248f*beCao;
   thapNhiDien.mangDinh[19].z = -0.79465445f*beRong;

   thapNhiDien.mangDinh[20].x = -0.57735022f*beDai;
   thapNhiDien.mangDinh[20].y = 0.18759248f*beCao;
   thapNhiDien.mangDinh[20].z = -0.79465445f*beRong;
   
   thapNhiDien.mangDinh[21].x = -0.93417235f*beDai;
   thapNhiDien.mangDinh[21].y = 0.18759248f*beCao;
   thapNhiDien.mangDinh[21].z = 0.30353102f*beRong;
   
   // ----
   thapNhiDien.mangDinh[22].x = 0.57735022f*beDai;
   thapNhiDien.mangDinh[22].y = -0.18759248f*beCao;
   thapNhiDien.mangDinh[22].z = 0.79465445f*beRong;
   
   thapNhiDien.mangDinh[23].x = 0.93417235f*beDai;
   thapNhiDien.mangDinh[23].y = -0.18759248f*beCao;
   thapNhiDien.mangDinh[23].z = -0.30353102f*beRong;
   
   thapNhiDien.mangDinh[24].x = 0.0f;
   thapNhiDien.mangDinh[24].y = -0.18759248f*beCao;
   thapNhiDien.mangDinh[24].z = -0.98224695f*beRong;

   thapNhiDien.mangDinh[25].x = -0.93417235f*beDai;
   thapNhiDien.mangDinh[25].y = -0.18759248f*beCao;
   thapNhiDien.mangDinh[25].z = -0.30353102f*beRong;
   
   thapNhiDien.mangDinh[26].x = -0.57735022f*beDai;
   thapNhiDien.mangDinh[26].y = -0.18759248f*beCao;
   thapNhiDien.mangDinh[26].z = 0.79465445f*beRong;

   // ---- rìa mặt dưới
   thapNhiDien.mangDinh[27].x = 0.35682211f*beDai;
   thapNhiDien.mangDinh[27].y = -0.79465451f*beCao;
   thapNhiDien.mangDinh[27].z = 0.49112344f*beRong;
   
   thapNhiDien.mangDinh[28].x = 0.57735022f*beDai;
   thapNhiDien.mangDinh[28].y = -0.79465451f*beCao;
   thapNhiDien.mangDinh[28].z = -0.18759247f*beRong;
   
   thapNhiDien.mangDinh[29].x = 0.0f;
   thapNhiDien.mangDinh[29].y = -0.79465451f*beCao;
   thapNhiDien.mangDinh[29].z = -0.60706196f*beRong;
   
   thapNhiDien.mangDinh[30].x = -0.57735022f*beDai;
   thapNhiDien.mangDinh[30].y = -0.79465451f*beCao;
   thapNhiDien.mangDinh[30].z = -0.18759247f*beRong;
   
   thapNhiDien.mangDinh[31].x = -0.35682211f*beDai;
   thapNhiDien.mangDinh[31].y = -0.79465451f*beCao;
   thapNhiDien.mangDinh[31].z = 0.49112344f*beRong;

   // ---- mảng tam giác
   // ngũ giác trên
   thapNhiDien.mangTamGiac[0].dinh0 = 0;
   thapNhiDien.mangTamGiac[0].dinh1 = 12;
   thapNhiDien.mangTamGiac[0].dinh2 = 13;
   
   thapNhiDien.mangTamGiac[1].dinh0 = 0;
   thapNhiDien.mangTamGiac[1].dinh1 = 13;
   thapNhiDien.mangTamGiac[1].dinh2 = 14;
   
   thapNhiDien.mangTamGiac[2].dinh0 = 0;
   thapNhiDien.mangTamGiac[2].dinh1 = 14;
   thapNhiDien.mangTamGiac[2].dinh2 = 15;
   
   thapNhiDien.mangTamGiac[3].dinh0 = 0;
   thapNhiDien.mangTamGiac[3].dinh1 = 15;
   thapNhiDien.mangTamGiac[3].dinh2 = 16;

   thapNhiDien.mangTamGiac[4].dinh0 = 0;
   thapNhiDien.mangTamGiac[4].dinh1 = 16;
   thapNhiDien.mangTamGiac[4].dinh2 = 12;
   // ----
   thapNhiDien.mangTamGiac[5].dinh0 = 1;
   thapNhiDien.mangTamGiac[5].dinh1 = 13;
   thapNhiDien.mangTamGiac[5].dinh2 = 12;
   
   thapNhiDien.mangTamGiac[6].dinh0 = 1;
   thapNhiDien.mangTamGiac[6].dinh1 = 12;
   thapNhiDien.mangTamGiac[6].dinh2 = 17;
   
   thapNhiDien.mangTamGiac[7].dinh0 = 1;
   thapNhiDien.mangTamGiac[7].dinh1 = 17;
   thapNhiDien.mangTamGiac[7].dinh2 = 22;
   
   thapNhiDien.mangTamGiac[8].dinh0 = 1;
   thapNhiDien.mangTamGiac[8].dinh1 = 22;
   thapNhiDien.mangTamGiac[8].dinh2 = 18;
   
   thapNhiDien.mangTamGiac[9].dinh0 = 1;
   thapNhiDien.mangTamGiac[9].dinh1 = 18;
   thapNhiDien.mangTamGiac[9].dinh2 = 13;
   // ----
   thapNhiDien.mangTamGiac[10].dinh0 = 2;
   thapNhiDien.mangTamGiac[10].dinh1 = 14;
   thapNhiDien.mangTamGiac[10].dinh2 = 13;
   
   thapNhiDien.mangTamGiac[11].dinh0 = 2;
   thapNhiDien.mangTamGiac[11].dinh1 = 13;
   thapNhiDien.mangTamGiac[11].dinh2 = 18;

   thapNhiDien.mangTamGiac[12].dinh0 = 2;
   thapNhiDien.mangTamGiac[12].dinh1 = 18;
   thapNhiDien.mangTamGiac[12].dinh2 = 23;
   
   thapNhiDien.mangTamGiac[13].dinh0 = 2;
   thapNhiDien.mangTamGiac[13].dinh1 = 23;
   thapNhiDien.mangTamGiac[13].dinh2 = 19;

   thapNhiDien.mangTamGiac[14].dinh0 = 2;
   thapNhiDien.mangTamGiac[14].dinh1 = 19;
   thapNhiDien.mangTamGiac[14].dinh2 = 14;
   // ----
   thapNhiDien.mangTamGiac[15].dinh0 = 3;
   thapNhiDien.mangTamGiac[15].dinh1 = 15;
   thapNhiDien.mangTamGiac[15].dinh2 = 14;

   thapNhiDien.mangTamGiac[16].dinh0 = 3;
   thapNhiDien.mangTamGiac[16].dinh1 = 14;
   thapNhiDien.mangTamGiac[16].dinh2 = 19;
   
   thapNhiDien.mangTamGiac[17].dinh0 = 3;
   thapNhiDien.mangTamGiac[17].dinh1 = 19;
   thapNhiDien.mangTamGiac[17].dinh2 = 24;
   
   thapNhiDien.mangTamGiac[18].dinh0 = 3;
   thapNhiDien.mangTamGiac[18].dinh1 = 24;
   thapNhiDien.mangTamGiac[18].dinh2 = 20;
   
   thapNhiDien.mangTamGiac[19].dinh0 = 3;
   thapNhiDien.mangTamGiac[19].dinh1 = 20;
   thapNhiDien.mangTamGiac[19].dinh2 = 15;
   // ----
   thapNhiDien.mangTamGiac[20].dinh0 = 4;
   thapNhiDien.mangTamGiac[20].dinh1 = 16;
   thapNhiDien.mangTamGiac[20].dinh2 = 15;
   
   thapNhiDien.mangTamGiac[21].dinh0 = 4;
   thapNhiDien.mangTamGiac[21].dinh1 = 15;
   thapNhiDien.mangTamGiac[21].dinh2 = 20;

   thapNhiDien.mangTamGiac[22].dinh0 = 4;
   thapNhiDien.mangTamGiac[22].dinh1 = 20;
   thapNhiDien.mangTamGiac[22].dinh2 = 25;

   thapNhiDien.mangTamGiac[23].dinh0 = 4;
   thapNhiDien.mangTamGiac[23].dinh1 = 25;
   thapNhiDien.mangTamGiac[23].dinh2 = 21;

   thapNhiDien.mangTamGiac[24].dinh0 = 4;
   thapNhiDien.mangTamGiac[24].dinh1 = 21;
   thapNhiDien.mangTamGiac[24].dinh2 = 16;
   // ----
   thapNhiDien.mangTamGiac[25].dinh0 = 5;
   thapNhiDien.mangTamGiac[25].dinh1 = 12;
   thapNhiDien.mangTamGiac[25].dinh2 = 16;

   thapNhiDien.mangTamGiac[26].dinh0 = 5;
   thapNhiDien.mangTamGiac[26].dinh1 = 16;
   thapNhiDien.mangTamGiac[26].dinh2 = 21;

   thapNhiDien.mangTamGiac[27].dinh0 = 5;
   thapNhiDien.mangTamGiac[27].dinh1 = 21;
   thapNhiDien.mangTamGiac[27].dinh2 = 26;

   thapNhiDien.mangTamGiac[28].dinh0 = 5;
   thapNhiDien.mangTamGiac[28].dinh1 = 26;
   thapNhiDien.mangTamGiac[28].dinh2 = 17;
   
   thapNhiDien.mangTamGiac[29].dinh0 = 5;
   thapNhiDien.mangTamGiac[29].dinh1 = 17;
   thapNhiDien.mangTamGiac[29].dinh2 = 12;
   // ====
   thapNhiDien.mangTamGiac[30].dinh0 = 6;
   thapNhiDien.mangTamGiac[30].dinh1 = 31;
   thapNhiDien.mangTamGiac[30].dinh2 = 27;
   
   thapNhiDien.mangTamGiac[31].dinh0 = 6;
   thapNhiDien.mangTamGiac[31].dinh1 = 26;
   thapNhiDien.mangTamGiac[31].dinh2 = 31;

   thapNhiDien.mangTamGiac[32].dinh0 = 6;
   thapNhiDien.mangTamGiac[32].dinh1 = 17;
   thapNhiDien.mangTamGiac[32].dinh2 = 26;
   
   thapNhiDien.mangTamGiac[33].dinh0 = 6;
   thapNhiDien.mangTamGiac[33].dinh1 = 22;
   thapNhiDien.mangTamGiac[33].dinh2 = 17;
   
   thapNhiDien.mangTamGiac[34].dinh0 = 6;
   thapNhiDien.mangTamGiac[34].dinh1 = 27;
   thapNhiDien.mangTamGiac[34].dinh2 = 22;
   // ----
   thapNhiDien.mangTamGiac[35].dinh0 = 7;
   thapNhiDien.mangTamGiac[35].dinh1 = 27;
   thapNhiDien.mangTamGiac[35].dinh2 = 28;
   
   thapNhiDien.mangTamGiac[36].dinh0 = 7;
   thapNhiDien.mangTamGiac[36].dinh1 = 22;
   thapNhiDien.mangTamGiac[36].dinh2 = 27;

   thapNhiDien.mangTamGiac[37].dinh0 = 7;
   thapNhiDien.mangTamGiac[37].dinh1 = 18;
   thapNhiDien.mangTamGiac[37].dinh2 = 22;

   thapNhiDien.mangTamGiac[38].dinh0 = 7;
   thapNhiDien.mangTamGiac[38].dinh1 = 23;
   thapNhiDien.mangTamGiac[38].dinh2 = 18;
   
   thapNhiDien.mangTamGiac[39].dinh0 = 7;
   thapNhiDien.mangTamGiac[39].dinh1 = 28;
   thapNhiDien.mangTamGiac[39].dinh2 = 23;
   // ----
   thapNhiDien.mangTamGiac[40].dinh0 = 8;
   thapNhiDien.mangTamGiac[40].dinh1 = 28;
   thapNhiDien.mangTamGiac[40].dinh2 = 29;

   thapNhiDien.mangTamGiac[41].dinh0 = 8;
   thapNhiDien.mangTamGiac[41].dinh1 = 23;
   thapNhiDien.mangTamGiac[41].dinh2 = 28;

   thapNhiDien.mangTamGiac[42].dinh0 = 8;
   thapNhiDien.mangTamGiac[42].dinh1 = 19;
   thapNhiDien.mangTamGiac[42].dinh2 = 23;

   thapNhiDien.mangTamGiac[43].dinh0 = 8;
   thapNhiDien.mangTamGiac[43].dinh1 = 24;
   thapNhiDien.mangTamGiac[43].dinh2 = 19;
   
   thapNhiDien.mangTamGiac[44].dinh0 = 8;
   thapNhiDien.mangTamGiac[44].dinh1 = 29;
   thapNhiDien.mangTamGiac[44].dinh2 = 24;
   // ----
   thapNhiDien.mangTamGiac[45].dinh0 = 9;
   thapNhiDien.mangTamGiac[45].dinh1 = 29;
   thapNhiDien.mangTamGiac[45].dinh2 = 30;
   
   thapNhiDien.mangTamGiac[46].dinh0 = 9;
   thapNhiDien.mangTamGiac[46].dinh1 = 24;
   thapNhiDien.mangTamGiac[46].dinh2 = 29;

   thapNhiDien.mangTamGiac[47].dinh0 = 9;
   thapNhiDien.mangTamGiac[47].dinh1 = 20;
   thapNhiDien.mangTamGiac[47].dinh2 = 24;

   thapNhiDien.mangTamGiac[48].dinh0 = 9;
   thapNhiDien.mangTamGiac[48].dinh1 = 25;
   thapNhiDien.mangTamGiac[48].dinh2 = 20;
   
   thapNhiDien.mangTamGiac[49].dinh0 = 9;
   thapNhiDien.mangTamGiac[49].dinh1 = 30;
   thapNhiDien.mangTamGiac[49].dinh2 = 25;
   // ----
   thapNhiDien.mangTamGiac[50].dinh0 = 10;
   thapNhiDien.mangTamGiac[50].dinh1 = 30;
   thapNhiDien.mangTamGiac[50].dinh2 = 31;
   
   thapNhiDien.mangTamGiac[51].dinh0 = 10;
   thapNhiDien.mangTamGiac[51].dinh1 = 25;
   thapNhiDien.mangTamGiac[51].dinh2 = 30;
   
   thapNhiDien.mangTamGiac[52].dinh0 = 10;
   thapNhiDien.mangTamGiac[52].dinh1 = 21;
   thapNhiDien.mangTamGiac[52].dinh2 = 25;

   thapNhiDien.mangTamGiac[53].dinh0 = 10;
   thapNhiDien.mangTamGiac[53].dinh1 = 26;
   thapNhiDien.mangTamGiac[53].dinh2 = 21;

   thapNhiDien.mangTamGiac[54].dinh0 = 10;
   thapNhiDien.mangTamGiac[54].dinh1 = 31;
   thapNhiDien.mangTamGiac[54].dinh2 = 26;
   // ----
   thapNhiDien.mangTamGiac[55].dinh0 = 11;
   thapNhiDien.mangTamGiac[55].dinh1 = 28;
   thapNhiDien.mangTamGiac[55].dinh2 = 27;
   
   thapNhiDien.mangTamGiac[56].dinh0 = 11;
   thapNhiDien.mangTamGiac[56].dinh1 = 29;
   thapNhiDien.mangTamGiac[56].dinh2 = 28;

   thapNhiDien.mangTamGiac[57].dinh0 = 11;
   thapNhiDien.mangTamGiac[57].dinh1 = 30;
   thapNhiDien.mangTamGiac[57].dinh2 = 29;

   thapNhiDien.mangTamGiac[58].dinh0 = 11;
   thapNhiDien.mangTamGiac[58].dinh1 = 31;
   thapNhiDien.mangTamGiac[58].dinh2 = 30;
   
   thapNhiDien.mangTamGiac[59].dinh0 = 11;
   thapNhiDien.mangTamGiac[59].dinh1 = 27;
   thapNhiDien.mangTamGiac[59].dinh2 = 31;

   baoBi->gocCucTieu.x = -0.98224695f*beRong;
   baoBi->gocCucDai.x = 0.98224695f*beRong;
   baoBi->gocCucTieu.y = -0.79465451f*beCao;
   baoBi->gocCucDai.y = 0.79465451f*beCao;
   baoBi->gocCucTieu.z = -0.93417235f*beDai;
   baoBi->gocCucDai.z = 0.93417235f*beDai;
   
   return thapNhiDien;
}


#pragma mark ---- Kim Tư Tháp
KimTuThap datKimTuThap( float beRong, float beCao, float beDai, BaoBi *baoBi ) {
   
   KimTuThap kimTuThap;
   
   kimTuThap.soLuongTamGiac = 6;
   
   float nuaBeRong = 0.5f*beRong;
   float nuaBeCao = 0.5f*beCao;
   float nuaBeDai = 0.5f*beDai;

   // ---- đỉnh cực
   kimTuThap.mangDinh[0].x = 0.0f;
   kimTuThap.mangDinh[0].y = nuaBeCao;
   kimTuThap.mangDinh[0].z = 0.0f;
   // ---- bốn đỉnh đấy
   kimTuThap.mangDinh[1].x = nuaBeRong;
   kimTuThap.mangDinh[1].y = -nuaBeCao;
   kimTuThap.mangDinh[1].z = nuaBeDai;
   
   kimTuThap.mangDinh[2].x = nuaBeRong;
   kimTuThap.mangDinh[2].y = -nuaBeCao;
   kimTuThap.mangDinh[2].z = -nuaBeDai;
   
   kimTuThap.mangDinh[3].x = -nuaBeRong;
   kimTuThap.mangDinh[3].y = -nuaBeCao;
   kimTuThap.mangDinh[3].z = -nuaBeDai;
   
   kimTuThap.mangDinh[4].x = -nuaBeRong;
   kimTuThap.mangDinh[4].y = -nuaBeCao;
   kimTuThap.mangDinh[4].z = nuaBeDai;
   
   // ---- mảng tam giác
   // 4 mặt trên
   kimTuThap.mangTamGiac[0].dinh0 = 0;
   kimTuThap.mangTamGiac[0].dinh1 = 1;
   kimTuThap.mangTamGiac[0].dinh2 = 2;
   
   kimTuThap.mangTamGiac[1].dinh0 = 0;
   kimTuThap.mangTamGiac[1].dinh1 = 2;
   kimTuThap.mangTamGiac[1].dinh2 = 3;
   
   kimTuThap.mangTamGiac[2].dinh0 = 0;
   kimTuThap.mangTamGiac[2].dinh1 = 3;
   kimTuThap.mangTamGiac[2].dinh2 = 4;
   
   kimTuThap.mangTamGiac[3].dinh0 = 0;
   kimTuThap.mangTamGiac[3].dinh1 = 4;
   kimTuThap.mangTamGiac[3].dinh2 = 1;
   // 2 tam giác cho đấy
   kimTuThap.mangTamGiac[4].dinh0 = 1;
   kimTuThap.mangTamGiac[4].dinh1 = 3;
   kimTuThap.mangTamGiac[4].dinh2 = 2;
   
   kimTuThap.mangTamGiac[5].dinh0 = 3;
   kimTuThap.mangTamGiac[5].dinh1 = 1;
   kimTuThap.mangTamGiac[5].dinh2 = 4;
   
   baoBi->gocCucTieu.x = -nuaBeRong;
   baoBi->gocCucDai.x = nuaBeRong;
   baoBi->gocCucTieu.y = -nuaBeCao;
   baoBi->gocCucDai.y = nuaBeCao;
   baoBi->gocCucTieu.z = -nuaBeDai;
   baoBi->gocCucDai.z = nuaBeDai;

   return kimTuThap;
}

#pragma mark ---- Dốc
// hướng lên -z
Doc datDoc( float beRong, float beCao, float beDai, BaoBi *baoBiVT ) {
   
   Doc doc;
   
   doc.soLuongTamGiac = 8;
   float nuaBeRong = 0.5f*beRong;
   float nuaBeCao = 0.5f*beCao;
   float nuaBeDai = 0.5f*beDai;

   // ---- hai đỉnh cao
   doc.mangDinh[0].x = nuaBeRong;
   doc.mangDinh[0].y = nuaBeCao;
   doc.mangDinh[0].z = nuaBeDai;

   doc.mangDinh[1].x = -nuaBeRong;
   doc.mangDinh[1].y = nuaBeCao;
   doc.mangDinh[1].z = nuaBeDai;
   // ---- bốn đỉnh đấy
   doc.mangDinh[2].x = nuaBeRong;
   doc.mangDinh[2].y = -nuaBeCao;
   doc.mangDinh[2].z = nuaBeDai;
   
   doc.mangDinh[3].x = -nuaBeRong;
   doc.mangDinh[3].y = -nuaBeCao;
   doc.mangDinh[3].z = nuaBeDai;
   
   doc.mangDinh[4].x = -nuaBeRong;
   doc.mangDinh[4].y = -nuaBeCao;
   doc.mangDinh[4].z = -nuaBeDai;

   doc.mangDinh[5].x = nuaBeRong;
   doc.mangDinh[5].y = -nuaBeCao;
   doc.mangDinh[5].z = -nuaBeDai;
   
   // ---- mảng tam giác
   // mặt trên nghiêng
   doc.mangTamGiac[0].dinh0 = 1;
   doc.mangTamGiac[0].dinh1 = 0;
   doc.mangTamGiac[0].dinh2 = 5;
   
   doc.mangTamGiac[1].dinh0 = 5;
   doc.mangTamGiac[1].dinh1 = 4;
   doc.mangTamGiac[1].dinh2 = 1;
   // mặt +x
   doc.mangTamGiac[2].dinh0 = 0;
   doc.mangTamGiac[2].dinh1 = 2;
   doc.mangTamGiac[2].dinh2 = 5;
   // mặt -x
   doc.mangTamGiac[3].dinh0 = 4;
   doc.mangTamGiac[3].dinh1 = 3;
   doc.mangTamGiac[3].dinh2 = 1;
   // mặt +z
   doc.mangTamGiac[4].dinh0 = 0;
   doc.mangTamGiac[4].dinh1 = 1;
   doc.mangTamGiac[4].dinh2 = 3;
   
   doc.mangTamGiac[5].dinh0 = 2;
   doc.mangTamGiac[5].dinh1 = 0;
   doc.mangTamGiac[5].dinh2 = 3;
   // mặt -y
   doc.mangTamGiac[6].dinh0 = 5;
   doc.mangTamGiac[6].dinh1 = 2;
   doc.mangTamGiac[6].dinh2 = 3;
   
   doc.mangTamGiac[7].dinh0 = 3;
   doc.mangTamGiac[7].dinh1 = 4;
   doc.mangTamGiac[7].dinh2 = 5;
   // 
   baoBiVT->gocCucTieu.x = -nuaBeRong;
   baoBiVT->gocCucDai.x = nuaBeRong;
   baoBiVT->gocCucTieu.y = -nuaBeCao;
   baoBiVT->gocCucDai.y = nuaBeCao;
   baoBiVT->gocCucTieu.z = -nuaBeDai;
   baoBiVT->gocCucDai.z = nuaBeDai;
   
   return doc;
}

#pragma mark ---- Nhị Thập Điện
NhiThapDien datNhiThapDien( float beRong, float beCao, float beDai, BaoBi *baoBiVT ) {
   NhiThapDien nhiThapDien;
   nhiThapDien.soLuongTamGiac = 20;
   
   float nuaBeRong = 0.5f*beRong;
   float nuaBeCao = 0.5f*beCao;
   float nuaBeDai = 0.5f*beDai;

   // ---- đỉnh trên
   nhiThapDien.mangDinh[0].x = 0.00000f;   nhiThapDien.mangDinh[0].y = 0.00000f;   nhiThapDien.mangDinh[0].z = 1.00000f*nuaBeDai;
   
   nhiThapDien.mangDinh[1].x = -0.72360*nuaBeRong;  nhiThapDien.mangDinh[1].y = -0.52572f*nuaBeCao;  nhiThapDien.mangDinh[1].z = 0.44721f*nuaBeDai;
   nhiThapDien.mangDinh[2].x = 0.27639f*nuaBeRong;   nhiThapDien.mangDinh[2].y = -0.85064f*nuaBeCao;  nhiThapDien.mangDinh[2].z = 0.44721f*nuaBeDai;
   nhiThapDien.mangDinh[3].x = 0.89442f*nuaBeRong;   nhiThapDien.mangDinh[3].y = 0.00000;   nhiThapDien.mangDinh[3].z = 0.44721f*nuaBeDai;
   nhiThapDien.mangDinh[4].x = 0.27639f*nuaBeRong;   nhiThapDien.mangDinh[4].y = 0.85064f*nuaBeCao;   nhiThapDien.mangDinh[4].z = 0.44721f*nuaBeDai;
   nhiThapDien.mangDinh[5].x = -0.72360*nuaBeRong;  nhiThapDien.mangDinh[5].y = 0.52572f*nuaBeCao;   nhiThapDien.mangDinh[5].z = 0.44721f*nuaBeDai;
   
   nhiThapDien.mangDinh[6].x = -0.89442f*nuaBeRong;  nhiThapDien.mangDinh[6].y = 0.00000;   nhiThapDien.mangDinh[6].z = -0.44721f*nuaBeDai;
   nhiThapDien.mangDinh[7].x = -0.27639f*nuaBeRong;  nhiThapDien.mangDinh[7].y = -0.85064f*nuaBeCao;  nhiThapDien.mangDinh[7].z = -0.44721f*nuaBeDai;
   nhiThapDien.mangDinh[8].x = 0.72360*nuaBeRong;   nhiThapDien.mangDinh[8].y = -0.52572f*nuaBeCao;  nhiThapDien.mangDinh[8].z = -0.44721f*nuaBeDai;
   nhiThapDien.mangDinh[9].x = 0.72360*nuaBeRong;   nhiThapDien.mangDinh[9].y = 0.52572f*nuaBeCao;   nhiThapDien.mangDinh[9].z = -0.44721f*nuaBeDai;
   nhiThapDien.mangDinh[10].x = -0.27639f*nuaBeRong;  nhiThapDien.mangDinh[10].y = 0.85064f*nuaBeCao;   nhiThapDien.mangDinh[10].z = -0.44721f*nuaBeDai;
   // ---- đỉnh dưới
   nhiThapDien.mangDinh[11].x = 0.00000f;   nhiThapDien.mangDinh[11].y = 0.00000f;   nhiThapDien.mangDinh[11].z = -1.00000f*nuaBeDai;
   
   // ---- mảng tam giác (đỉnh gai trước, hai điểm của dấy tam giác sau; mỗi gai có ba tam giác)
   nhiThapDien.mangTamGiac[0].dinh0 = 0;   nhiThapDien.mangTamGiac[0].dinh1 = 1;   nhiThapDien.mangTamGiac[0].dinh2 = 2;
   nhiThapDien.mangTamGiac[1].dinh0 = 0;   nhiThapDien.mangTamGiac[1].dinh1 = 2;   nhiThapDien.mangTamGiac[1].dinh2 = 3;
   nhiThapDien.mangTamGiac[2].dinh0 = 0;   nhiThapDien.mangTamGiac[2].dinh1 = 3;   nhiThapDien.mangTamGiac[2].dinh2 = 4;
   nhiThapDien.mangTamGiac[3].dinh0 = 0;   nhiThapDien.mangTamGiac[3].dinh1 = 4;   nhiThapDien.mangTamGiac[3].dinh2 = 5;
   nhiThapDien.mangTamGiac[4].dinh0 = 0;   nhiThapDien.mangTamGiac[4].dinh1 = 5;   nhiThapDien.mangTamGiac[4].dinh2 = 1;

   nhiThapDien.mangTamGiac[5].dinh0 = 6;   nhiThapDien.mangTamGiac[5].dinh1 = 1;   nhiThapDien.mangTamGiac[5].dinh2 = 5;
   nhiThapDien.mangTamGiac[6].dinh0 = 7;   nhiThapDien.mangTamGiac[6].dinh1 = 2;   nhiThapDien.mangTamGiac[6].dinh2 = 1;
   nhiThapDien.mangTamGiac[7].dinh0 = 8;   nhiThapDien.mangTamGiac[7].dinh1 = 3;   nhiThapDien.mangTamGiac[7].dinh2 = 2;
   nhiThapDien.mangTamGiac[8].dinh0 = 9;   nhiThapDien.mangTamGiac[8].dinh1 = 4;   nhiThapDien.mangTamGiac[8].dinh2 = 3;
   nhiThapDien.mangTamGiac[9].dinh0 = 10;   nhiThapDien.mangTamGiac[9].dinh1 = 5;   nhiThapDien.mangTamGiac[9].dinh2 = 4;

   nhiThapDien.mangTamGiac[10].dinh0 = 6;  nhiThapDien.mangTamGiac[10].dinh1 = 7;  nhiThapDien.mangTamGiac[10].dinh2 = 1;
   nhiThapDien.mangTamGiac[11].dinh0 = 7;  nhiThapDien.mangTamGiac[11].dinh1 = 8;  nhiThapDien.mangTamGiac[11].dinh2 = 2;
   nhiThapDien.mangTamGiac[12].dinh0 = 8;  nhiThapDien.mangTamGiac[12].dinh1 = 9;  nhiThapDien.mangTamGiac[12].dinh2 = 3;
   nhiThapDien.mangTamGiac[13].dinh0 = 9;  nhiThapDien.mangTamGiac[13].dinh1 = 10;  nhiThapDien.mangTamGiac[13].dinh2 = 4;
   nhiThapDien.mangTamGiac[14].dinh0 = 10;  nhiThapDien.mangTamGiac[14].dinh1 = 6;  nhiThapDien.mangTamGiac[14].dinh2 = 5;
   // ----
   nhiThapDien.mangTamGiac[15].dinh0 = 6;  nhiThapDien.mangTamGiac[15].dinh1 = 11;  nhiThapDien.mangTamGiac[15].dinh2 = 7;
   nhiThapDien.mangTamGiac[16].dinh0 = 7;  nhiThapDien.mangTamGiac[16].dinh1 = 11;  nhiThapDien.mangTamGiac[16].dinh2 = 8;
   nhiThapDien.mangTamGiac[17].dinh0 = 8;  nhiThapDien.mangTamGiac[17].dinh1 = 11;  nhiThapDien.mangTamGiac[17].dinh2 = 9;
   nhiThapDien.mangTamGiac[18].dinh0 = 9;  nhiThapDien.mangTamGiac[18].dinh1 = 11;   nhiThapDien.mangTamGiac[18].dinh2 = 10;
   nhiThapDien.mangTamGiac[19].dinh0 = 10;  nhiThapDien.mangTamGiac[19].dinh1 = 11;   nhiThapDien.mangTamGiac[19].dinh2 = 6;

   // ---- bao bì
   baoBiVT->gocCucTieu.x = -0.44721f*nuaBeRong;
   baoBiVT->gocCucDai.x = 0.44721f*nuaBeRong;
   baoBiVT->gocCucTieu.y = -0.42532f*nuaBeCao;
   baoBiVT->gocCucDai.y = 0.42532f*nuaBeCao;
   baoBiVT->gocCucTieu.z = -0.50000f*nuaBeDai;
   baoBiVT->gocCucDai.z = 0.50000f*nuaBeDai;
   return nhiThapDien;
}


#pragma mark ---- Sao gai     XÀI bán kính 1,88
SaoGai datSaoGai( BaoBi *baoBiVT ) {
   SaoGai saoGai;
   saoGai.soLuongTamGiac = 60;
   // ---- các gai
   saoGai.mangDinh[0].x = -1.15180f;   saoGai.mangDinh[0].y = 0.00000f;    saoGai.mangDinh[0].z = 1.50772f;
   saoGai.mangDinh[1].x = -0.35593f;   saoGai.mangDinh[1].y = -1.09542f;   saoGai.mangDinh[1].z = 1.50772f;
   saoGai.mangDinh[2].x = 0.93182f;    saoGai.mangDinh[2].y = -0.67702f;   saoGai.mangDinh[2].z = 1.50772f;
   saoGai.mangDinh[3].x = 0.93182f;    saoGai.mangDinh[3].y = 0.67702f;    saoGai.mangDinh[3].z = 1.50772f;
   saoGai.mangDinh[4].x = -0.35593f;   saoGai.mangDinh[4].y = 1.09542f;    saoGai.mangDinh[4].z = 1.50772f;
   
   saoGai.mangDinh[5].x = -1.86364f;   saoGai.mangDinh[5].y = 0.00000f;    saoGai.mangDinh[5].z = 0.35592f;
   saoGai.mangDinh[6].x = -0.57590f;   saoGai.mangDinh[6].y = -1.77242f;   saoGai.mangDinh[6].z = 0.35592f;
   saoGai.mangDinh[7].x = 1.50771f;    saoGai.mangDinh[7].y = -1.09543f;   saoGai.mangDinh[7].z = 0.35592f;
   saoGai.mangDinh[8].x = 1.50771f;    saoGai.mangDinh[8].y = 1.09543f;    saoGai.mangDinh[8].z = 0.35592f;
   saoGai.mangDinh[9].x = -0.57590f;   saoGai.mangDinh[9].y = 1.77242f;    saoGai.mangDinh[9].z = 0.35592f;

   saoGai.mangDinh[10].x = -1.50771f;  saoGai.mangDinh[10].y = -1.09543f;  saoGai.mangDinh[10].z = -0.35592f;
   saoGai.mangDinh[11].x = 0.57590f;   saoGai.mangDinh[11].y = -1.77242f;  saoGai.mangDinh[11].z = -0.35592f;
   saoGai.mangDinh[12].x = 1.86364f;   saoGai.mangDinh[12].y = 0.00000f;   saoGai.mangDinh[12].z = -0.35592f;
   saoGai.mangDinh[13].x = 0.57590f;   saoGai.mangDinh[13].y = 1.77242f;   saoGai.mangDinh[13].z = -0.35592f;
   saoGai.mangDinh[14].x = -1.50771f;  saoGai.mangDinh[14].y = 1.09543f;   saoGai.mangDinh[14].z = -0.35592f;

   saoGai.mangDinh[15].x = -0.93182f;  saoGai.mangDinh[15].y = -0.67702f;  saoGai.mangDinh[15].z = -1.50772f;
   saoGai.mangDinh[16].x = 0.35593f;   saoGai.mangDinh[16].y = -1.09542f;  saoGai.mangDinh[16].z = -1.50772f;
   saoGai.mangDinh[17].x = 1.15180f;   saoGai.mangDinh[17].y = 0.00000f;   saoGai.mangDinh[17].z = -1.50772f;
   saoGai.mangDinh[18].x = 0.35593f;   saoGai.mangDinh[18].y = 1.09542f;   saoGai.mangDinh[18].z = -1.50772f;
   saoGai.mangDinh[19].x = -0.93182f;  saoGai.mangDinh[19].y = 0.67702f;   saoGai.mangDinh[19].z = -1.50772f;

   // ==== nhị thập diện
   // ---- đỉnh trên
   saoGai.mangDinh[20].x = 0.00000f;   saoGai.mangDinh[20].y = 0.00000f;   saoGai.mangDinh[20].z = 0.50000f;
   
   saoGai.mangDinh[21].x = -0.36180f;  saoGai.mangDinh[21].y = -0.26286f;  saoGai.mangDinh[21].z = 0.22361f;
   saoGai.mangDinh[22].x = 0.13819f;   saoGai.mangDinh[22].y = -0.42532f;  saoGai.mangDinh[22].z = 0.22361f;
   saoGai.mangDinh[23].x = 0.44721f;   saoGai.mangDinh[23].y = 0.00000f;   saoGai.mangDinh[23].z = 0.22361f;
   saoGai.mangDinh[24].x = 0.13819f;   saoGai.mangDinh[24].y = 0.42532f;   saoGai.mangDinh[24].z = 0.22361f;
   saoGai.mangDinh[25].x = -0.36180f;  saoGai.mangDinh[25].y = 0.26286f;   saoGai.mangDinh[25].z = 0.22361f;

   saoGai.mangDinh[26].x = -0.44721f;  saoGai.mangDinh[26].y = 0.00000f;   saoGai.mangDinh[26].z = -0.22361f;
   saoGai.mangDinh[27].x = -0.13819f;  saoGai.mangDinh[27].y = -0.42532f;  saoGai.mangDinh[27].z = -0.22361f;
   saoGai.mangDinh[28].x = 0.36180f;   saoGai.mangDinh[28].y = -0.26286f;  saoGai.mangDinh[28].z = -0.22361f;
   saoGai.mangDinh[29].x = 0.36180f;   saoGai.mangDinh[29].y = 0.26286f;   saoGai.mangDinh[29].z = -0.22361f;
   saoGai.mangDinh[30].x = -0.13819f;  saoGai.mangDinh[30].y = 0.42532f;   saoGai.mangDinh[30].z = -0.22361f;
   // ---- đỉnh dưới
   saoGai.mangDinh[31].x = 0.00000f;   saoGai.mangDinh[31].y = 0.00000f;   saoGai.mangDinh[31].z = -0.50000f;
   
   // ---- mảng tam giác (đỉnh gai trước, hai điểm của dấy tam giác sau; mỗi gai có ba tam giác)
   saoGai.mangTamGiac[0].dinh0 = 0;   saoGai.mangTamGiac[0].dinh1 = 21;   saoGai.mangTamGiac[0].dinh2 = 20;
   saoGai.mangTamGiac[1].dinh0 = 0;   saoGai.mangTamGiac[1].dinh1 = 20;   saoGai.mangTamGiac[1].dinh2 = 25;
   saoGai.mangTamGiac[2].dinh0 = 0;   saoGai.mangTamGiac[2].dinh1 = 25;   saoGai.mangTamGiac[2].dinh2 = 21;

   saoGai.mangTamGiac[3].dinh0 = 1;   saoGai.mangTamGiac[3].dinh1 = 22;   saoGai.mangTamGiac[3].dinh2 = 20;
   saoGai.mangTamGiac[4].dinh0 = 1;   saoGai.mangTamGiac[4].dinh1 = 20;   saoGai.mangTamGiac[4].dinh2 = 21;
   saoGai.mangTamGiac[5].dinh0 = 1;   saoGai.mangTamGiac[5].dinh1 = 21;   saoGai.mangTamGiac[5].dinh2 = 22;

   saoGai.mangTamGiac[6].dinh0 = 2;   saoGai.mangTamGiac[6].dinh1 = 23;   saoGai.mangTamGiac[6].dinh2 = 20;
   saoGai.mangTamGiac[7].dinh0 = 2;   saoGai.mangTamGiac[7].dinh1 = 20;   saoGai.mangTamGiac[7].dinh2 = 22;
   saoGai.mangTamGiac[8].dinh0 = 2;   saoGai.mangTamGiac[8].dinh1 = 22;   saoGai.mangTamGiac[8].dinh2 = 23;

   saoGai.mangTamGiac[9].dinh0 = 3;   saoGai.mangTamGiac[9].dinh1 = 24;   saoGai.mangTamGiac[9].dinh2 = 20;
   saoGai.mangTamGiac[10].dinh0 = 3;  saoGai.mangTamGiac[10].dinh1 = 20;  saoGai.mangTamGiac[10].dinh2 = 23;
   saoGai.mangTamGiac[11].dinh0 = 3;  saoGai.mangTamGiac[11].dinh1 = 23;  saoGai.mangTamGiac[11].dinh2 = 24;

   saoGai.mangTamGiac[12].dinh0 = 4;  saoGai.mangTamGiac[12].dinh1 = 25;  saoGai.mangTamGiac[12].dinh2 = 20;
   saoGai.mangTamGiac[13].dinh0 = 4;  saoGai.mangTamGiac[13].dinh1 = 20;  saoGai.mangTamGiac[13].dinh2 = 24;
   saoGai.mangTamGiac[14].dinh0 = 4;  saoGai.mangTamGiac[14].dinh1 = 24;  saoGai.mangTamGiac[14].dinh2 = 25;
   // ----
   saoGai.mangTamGiac[15].dinh0 = 5;  saoGai.mangTamGiac[15].dinh1 = 21;  saoGai.mangTamGiac[15].dinh2 = 25;
   saoGai.mangTamGiac[16].dinh0 = 5;  saoGai.mangTamGiac[16].dinh1 = 25;  saoGai.mangTamGiac[16].dinh2 = 26;
   saoGai.mangTamGiac[17].dinh0 = 5;  saoGai.mangTamGiac[17].dinh1 = 26;  saoGai.mangTamGiac[17].dinh2 = 21;

   saoGai.mangTamGiac[18].dinh0 = 6;  saoGai.mangTamGiac[18].dinh1 = 22;   saoGai.mangTamGiac[18].dinh2 = 21;
   saoGai.mangTamGiac[19].dinh0 = 6;  saoGai.mangTamGiac[19].dinh1 = 21;   saoGai.mangTamGiac[19].dinh2 = 27;
   saoGai.mangTamGiac[20].dinh0 = 6;  saoGai.mangTamGiac[20].dinh1 = 27;   saoGai.mangTamGiac[20].dinh2 = 22;
   
   saoGai.mangTamGiac[21].dinh0 = 7;  saoGai.mangTamGiac[21].dinh1 = 23;  saoGai.mangTamGiac[21].dinh2 = 22;
   saoGai.mangTamGiac[22].dinh0 = 7;  saoGai.mangTamGiac[22].dinh1 = 22;  saoGai.mangTamGiac[22].dinh2 = 28;
   saoGai.mangTamGiac[23].dinh0 = 7;  saoGai.mangTamGiac[23].dinh1 = 28;  saoGai.mangTamGiac[23].dinh2 = 23;

   saoGai.mangTamGiac[24].dinh0 = 8; saoGai.mangTamGiac[24].dinh1 = 24;   saoGai.mangTamGiac[24].dinh2 = 23;
   saoGai.mangTamGiac[25].dinh0 = 8; saoGai.mangTamGiac[25].dinh1 = 23;   saoGai.mangTamGiac[25].dinh2 = 29;
   saoGai.mangTamGiac[26].dinh0 = 8; saoGai.mangTamGiac[26].dinh1 = 29;   saoGai.mangTamGiac[26].dinh2 = 24;

   saoGai.mangTamGiac[27].dinh0 = 9; saoGai.mangTamGiac[27].dinh1 = 25;   saoGai.mangTamGiac[27].dinh2 = 24;
   saoGai.mangTamGiac[28].dinh0 = 9; saoGai.mangTamGiac[28].dinh1 = 24;   saoGai.mangTamGiac[28].dinh2 = 30;
   saoGai.mangTamGiac[29].dinh0 = 9; saoGai.mangTamGiac[29].dinh1 = 30;   saoGai.mangTamGiac[29].dinh2 = 25;
   // ----
   saoGai.mangTamGiac[30].dinh0 = 10;  saoGai.mangTamGiac[30].dinh1 = 27;  saoGai.mangTamGiac[30].dinh2 = 21;
   saoGai.mangTamGiac[31].dinh0 = 10;  saoGai.mangTamGiac[31].dinh1 = 21;  saoGai.mangTamGiac[31].dinh2 = 26;
   saoGai.mangTamGiac[32].dinh0 = 10;  saoGai.mangTamGiac[32].dinh1 = 26;  saoGai.mangTamGiac[32].dinh2 = 27;

   saoGai.mangTamGiac[33].dinh0 = 11;  saoGai.mangTamGiac[33].dinh1 = 28;   saoGai.mangTamGiac[33].dinh2 = 22;
   saoGai.mangTamGiac[34].dinh0 = 11;  saoGai.mangTamGiac[34].dinh1 = 22;   saoGai.mangTamGiac[34].dinh2 = 27;
   saoGai.mangTamGiac[35].dinh0 = 11;  saoGai.mangTamGiac[35].dinh1 = 27;   saoGai.mangTamGiac[35].dinh2 = 28;

   saoGai.mangTamGiac[36].dinh0 = 12;  saoGai.mangTamGiac[36].dinh1 = 29;   saoGai.mangTamGiac[36].dinh2 = 23;
   saoGai.mangTamGiac[37].dinh0 = 12;  saoGai.mangTamGiac[37].dinh1 = 23;   saoGai.mangTamGiac[37].dinh2 = 28;
   saoGai.mangTamGiac[38].dinh0 = 12;  saoGai.mangTamGiac[38].dinh1 = 28;   saoGai.mangTamGiac[38].dinh2 = 29;
   
   saoGai.mangTamGiac[39].dinh0 = 13;  saoGai.mangTamGiac[39].dinh1 = 30;   saoGai.mangTamGiac[39].dinh2 = 24;
   saoGai.mangTamGiac[40].dinh0 = 13;  saoGai.mangTamGiac[40].dinh1 = 24;   saoGai.mangTamGiac[40].dinh2 = 29;
   saoGai.mangTamGiac[41].dinh0 = 13;  saoGai.mangTamGiac[41].dinh1 = 29;   saoGai.mangTamGiac[41].dinh2 = 30;

   saoGai.mangTamGiac[42].dinh0 = 14;  saoGai.mangTamGiac[42].dinh1 = 26;   saoGai.mangTamGiac[42].dinh2 = 25;
   saoGai.mangTamGiac[43].dinh0 = 14;  saoGai.mangTamGiac[43].dinh1 = 25;   saoGai.mangTamGiac[43].dinh2 = 30;
   saoGai.mangTamGiac[44].dinh0 = 14;  saoGai.mangTamGiac[44].dinh1 = 30;   saoGai.mangTamGiac[44].dinh2 = 26;
   // ----
   saoGai.mangTamGiac[45].dinh0 = 15;  saoGai.mangTamGiac[45].dinh1 = 27;    saoGai.mangTamGiac[45].dinh2 = 26;
   saoGai.mangTamGiac[46].dinh0 = 15;  saoGai.mangTamGiac[46].dinh1 = 26;    saoGai.mangTamGiac[46].dinh2 = 31;
   saoGai.mangTamGiac[47].dinh0 = 15;  saoGai.mangTamGiac[47].dinh1 = 31;    saoGai.mangTamGiac[47].dinh2 = 27;

   saoGai.mangTamGiac[48].dinh0 = 16;   saoGai.mangTamGiac[48].dinh1 = 28;   saoGai.mangTamGiac[48].dinh2 = 27;
   saoGai.mangTamGiac[49].dinh0 = 16;   saoGai.mangTamGiac[49].dinh1 = 27;   saoGai.mangTamGiac[49].dinh2 = 31;
   saoGai.mangTamGiac[50].dinh0 = 16;   saoGai.mangTamGiac[50].dinh1 = 31;   saoGai.mangTamGiac[50].dinh2 = 28;

   saoGai.mangTamGiac[51].dinh0 = 17;   saoGai.mangTamGiac[51].dinh1 = 29;   saoGai.mangTamGiac[51].dinh2 = 28;
   saoGai.mangTamGiac[52].dinh0 = 17;   saoGai.mangTamGiac[52].dinh1 = 28;   saoGai.mangTamGiac[52].dinh2 = 31;
   saoGai.mangTamGiac[53].dinh0 = 17;   saoGai.mangTamGiac[53].dinh1 = 31;   saoGai.mangTamGiac[53].dinh2 = 29;

   saoGai.mangTamGiac[54].dinh0 = 18;   saoGai.mangTamGiac[54].dinh1 = 30;   saoGai.mangTamGiac[54].dinh2 = 29;
   saoGai.mangTamGiac[55].dinh0 = 18;   saoGai.mangTamGiac[55].dinh1 = 29;   saoGai.mangTamGiac[55].dinh2 = 31;
   saoGai.mangTamGiac[56].dinh0 = 18;   saoGai.mangTamGiac[56].dinh1 = 31;   saoGai.mangTamGiac[56].dinh2 = 30;

   saoGai.mangTamGiac[57].dinh0 = 19;   saoGai.mangTamGiac[57].dinh1 = 26;   saoGai.mangTamGiac[57].dinh2 = 30;
   saoGai.mangTamGiac[58].dinh0 = 19;   saoGai.mangTamGiac[58].dinh1 = 30;   saoGai.mangTamGiac[58].dinh2 = 31;
   saoGai.mangTamGiac[59].dinh0 = 19;   saoGai.mangTamGiac[59].dinh1 = 31;   saoGai.mangTamGiac[59].dinh2 = 26;
   
   // ---- hợp quanh
   baoBiVT->gocCucTieu.x = -1.86364f;
   baoBiVT->gocCucDai.x = 1.86364f;
   baoBiVT->gocCucTieu.y = -1.77242f;
   baoBiVT->gocCucDai.y = 1.77242f;
   baoBiVT->gocCucTieu.z = -1.50722f;
   baoBiVT->gocCucDai.z = 1.50722f;
   return saoGai;
}

#pragma mark ---- Tam Giác
Vecto tinhPhapTuyenChoTamGiac( Vecto *tamGiac ) {
   
   Vecto canh_01;
   Vecto canh_02;

   canh_01.x = tamGiac[1].x - tamGiac[0].x;
   canh_01.y = tamGiac[1].y - tamGiac[0].y;
   canh_01.z = tamGiac[1].z - tamGiac[0].z;

   canh_02.x = tamGiac[2].x - tamGiac[0].x;
   canh_02.y = tamGiac[2].y - tamGiac[0].y;
   canh_02.z = tamGiac[2].z - tamGiac[0].z;

   Vecto phapTuyen = tichCoHuong( &canh_01, &canh_02 );
   donViHoa( &phapTuyen );
   
   return phapTuyen;
}

#define kCACH_TOI_THIEU 1e-5f

// ---- Giải thuật Möller–Trumbore cho xem tia có cắt tam gíac
float xemCatTamGiacMT( Vecto *mangDinhTamGiac, Tia *tia ) {
   
   float khoangCach = kVO_CUC;
   // ---- tính vectơ hai cạnh từ đỉnh 0 điểm
   Vecto canh_01;
   canh_01.x = mangDinhTamGiac[1].x - mangDinhTamGiac[0].x;
   canh_01.y = mangDinhTamGiac[1].y - mangDinhTamGiac[0].y;
   canh_01.z = mangDinhTamGiac[1].z - mangDinhTamGiac[0].z;
   
   Vecto canh_02;
   canh_02.x = mangDinhTamGiac[2].x - mangDinhTamGiac[0].x;
   canh_02.y = mangDinhTamGiac[2].y - mangDinhTamGiac[0].y;
   canh_02.z = mangDinhTamGiac[2].z - mangDinhTamGiac[0].z;

   // ---- tích có hướng tia với cạnh 02
   Vecto ketQua = tichCoHuong( &(tia->huong), &canh_02 );
   
   // ---- xem nếu tích vô hướng tia và vectơ vuông góc cùng hướng
   float tichVoHuong = ketQua.x*canh_01.x + ketQua.y*canh_01.y + ketQua.z*canh_01.z;
   
   // ---- songๆ hay gần song với mặt phẳng tam giác
   if( tichVoHuong < kCACH_TOI_THIEU && -tichVoHuong > kCACH_TOI_THIEU )
      return khoangCach;
   // ---- nếu tích vô hướng > 0.0f, tìm điểm cắt mẳt phẳng tam giác
   else {
      // ---- đảo nghịch tích vô hướng
      float daoNghichTichVoHuong = 1.0f/tichVoHuong;
      
      // ---- tính vectơ từ điểm nhìn đỉnh 0
      Vecto vectoDiemNhinDenDinh0;
      vectoDiemNhinDenDinh0.x = tia->goc.x - mangDinhTamGiac[0].x;
      vectoDiemNhinDenDinh0.y = tia->goc.y - mangDinhTamGiac[0].y;
      vectoDiemNhinDenDinh0.z = tia->goc.z - mangDinhTamGiac[0].z;
      
      // ---- tính tọa độ u
      float u = vectoDiemNhinDenDinh0.x*ketQua.x + vectoDiemNhinDenDinh0.y*ketQua.y +
      vectoDiemNhinDenDinh0.z*ketQua.z;
      u *= daoNghichTichVoHuong;
      
      // ---- nếu u < 0,0 hay u > 1,0, không cắt tam giác
      if( u < 0.0f || u > 1.0f)
         return khoangCach;
      else {
         // ---- tính tọa độ v
         ketQua = tichCoHuong( &vectoDiemNhinDenDinh0, &canh_01 );
         
         float v = tia->huong.x*ketQua.x + tia->huong.y*ketQua.y + tia->huong.z*ketQua.z;
         v *= daoNghichTichVoHuong;
         
         // ---- nếu v < 0,0 hay v > 1,0, không cắt tam giác
         if( v < 0.0f || (u + v) > 1.0f)
            khoangCach = kVO_CUC;
         
         else {
            float t = canh_02.x*ketQua.x + canh_02.y*ketQua.y + canh_02.z*ketQua.z;
            t *= daoNghichTichVoHuong;
            
            // ---- nếu t > CÁCH_TỐI_THIỂU
            if( t > 0.01f )
               khoangCach = t;
         }
         
      }
   }
   
   return khoangCach;
}

#pragma mark ---- Hủy Danh SáchVật Thể
void huyDanhSachVatThe( VatThe *danhSachVatThe, unsigned short soLuongVatThe ) {
   
   unsigned short chiSoVatThe = 0;
   while( chiSoVatThe < soLuongVatThe ) {
      if( (danhSachVatThe[chiSoVatThe].loai == kLOAI_VAT_THE__BOOL)
         || (danhSachVatThe[chiSoVatThe].loai == kLOAI_VAT_THE__GHEP) ) {
         free( danhSachVatThe[chiSoVatThe].danhSachVatThe );
      }
      chiSoVatThe++;
   }
}

#pragma mark ---- Ảnh
Anh taoAnhVoiCoKich( unsigned short beRong, unsigned short beCao, float coKichDiemAnh ) {
 
   Anh anh;
   anh.beRong = beRong;
   anh.beCao = beCao;
   anh.coKichDiemAnh = coKichDiemAnh;

   // ---- dành trí nhớ
   anh.kenhDo = malloc( sizeof( float)*beRong*beCao );
   anh.kenhLuc = malloc( sizeof( float)*beRong*beCao );
   anh.kenhXanh = malloc( sizeof( float)*beRong*beCao );
   anh.kenhDuc = malloc( sizeof( float)*beRong*beCao );
   //   float anh.kenhXa = malloc( sizeof( float)*beCao*beCao );

   return anh;
}

void xoaAnh( Anh *anh ) {
   
   anh->beRong = 0;
   anh->beCao = 0;
   
   free( anh->kenhDo );
   free( anh->kenhLuc );
   free( anh->kenhXanh );
   free( anh->kenhDuc );
//   free( anh->kenhXa );
}

#pragma mark ---- Tích Có Hướng
Vecto tichCoHuong( Vecto *vecto0, Vecto *vecto1 ) {
   Vecto ketQua;
   ketQua.x = vecto0->y*vecto1->z - vecto0->z*vecto1->y;
   ketQua.y = vecto0->z*vecto1->x - vecto0->x*vecto1->z;
   ketQua.z = vecto0->x*vecto1->y - vecto0->y*vecto1->x;
   return ketQua;
}

void donViHoa( Vecto *vecto ) {
   float doLon = vecto->x*vecto->x + vecto->y*vecto->y + vecto->z*vecto->z;
   if( doLon > 0.0f ) {
      doLon = sqrtf( doLon );
      vecto->x /= doLon;
      vecto->y /= doLon;
      vecto->z /= doLon;
   }
}

#pragma mark ---- Lưư Ảnh
void luuThongTinKenh_EXR( FILE *tep, unsigned char *danhSachKenh, unsigned char soLuongKenh, unsigned char kieuDuLieu );
void luuThongTinCuaSoDuLieu( FILE *tep, unsigned int beRong, unsigned int beCao );
void luuThongTinCuaSoChieu( FILE *tep, unsigned int beRong, unsigned int beCao );
void luuThoiGianKetXuat( FILE *tep, unsigned short thoiGianKetXuat );
void luuBangDuLieuAnh( FILE *tep, unsigned short beRong, unsigned short beCao, unsigned char soLuongKenh, unsigned char kieuDuLieu );
void chepDuLieuKenhFloat( unsigned char *dem, const float *kenh, unsigned short beRong );
void chepDuLieuKenhHalf( unsigned char *dem, const float *kenh, unsigned short beRong );
unsigned short doiFloatSangHalf( float soFloat );
void locDuLieuTrongDem(unsigned char *dem, unsigned int beDai, unsigned char *demLoc );
unsigned int nenZIP(unsigned char *dem, int beDaiDem, unsigned char *demNen, int beDaiDemNen );

/* Lưu Ảnh ZIP */
void luuAnhZIP( char *tenTep, Anh *anh, unsigned char kieuDuLieu, unsigned short thoiGianKetXuat ) {
   
   FILE *tep = fopen( tenTep, "wb" );
//   luuThongTinKenhEXR( unsigned short beRong, unsigned short beCao );
   // ---- mã số EXR
   fputc( 0x76, tep );
   fputc( 0x2f, tep );
   fputc( 0x31, tep );
   fputc( 0x01, tep );

   // ---- phiên bản 2 (chỉ phiên bản 2 được phát hành)
   unsigned int phienBan = 0x02;
   fputc( 0x02, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   
   unsigned short beRong = anh->beRong;
   unsigned short beCao = anh->beCao;

   // ---- thông cho các kênh
   unsigned char danhSachKenh[4] = {'B', 'G', 'R'};
   luuThongTinKenh_EXR( tep, danhSachKenh, 3, kieuDuLieu );
   
   // ---- nén
   fprintf( tep, "compression" );
   fputc( 0x00, tep );
   fprintf( tep, "compression" );
   fputc( 0x00, tep );
   fputc( 0x01, tep );   // bề dài dữ liệu
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );

   fputc( 0x03, tep );  // ZIP

   // ---- cửa sổ dữ liệu
   luuThongTinCuaSoDuLieu( tep, beRong, beCao );

   // ---- cửa sổ dữ liệu
   luuThongTinCuaSoChieu( tep, beRong, beCao );
   
   // ---- thứ tự hàng
   fprintf( tep, "lineOrder" );
   fputc( 0x00, tep );
   fprintf( tep, "lineOrder" );
   fputc( 0x00, tep );
   fputc( 0x01, tep );   // bề dài dữ liệu
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   
   fputc( 0x00, tep );  // từ nhỏ tới lớn
   
   // ---- tỉ số cạnh điểm ảnh
   fprintf( tep, "pixelAspectRatio" );
   fputc( 0x00, tep );
   fprintf( tep, "float" );
   fputc( 0x00, tep );
   fputc( 0x04, tep );   // bề dài dữ liệu
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   
   fputc( 0x00, tep );  // 1.0
   fputc( 0x00, tep );
   fputc( 0x80, tep );
   fputc( 0x3f, tep );
   
   luuThoiGianKetXuat( tep, thoiGianKetXuat );
   
   // ---- trung tâm cửa sổ màn
   fprintf( tep, "screenWindowCenter" );
   fputc( 0x00, tep );
   fprintf( tep, "v2f" );
   fputc( 0x00, tep );
   fputc( 0x08, tep );   // bề dài dữ liệu
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   // ---- tọa độ x (hoành độ)
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   // ---- tọa độ y (tung độ)
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   
   // ---- bề rộng cửa sổ màn
   fprintf( tep, "screenWindowWidth" );
   fputc( 0x00, tep );
   fprintf( tep, "float" );
   fputc( 0x00, tep );
   fputc( 0x04, tep );   // bề dài dữ liệu
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );

   fputc( 0x00, tep );   // 1.0f
   fputc( 0x00, tep );
   fputc( 0x80, tep );
   fputc( 0x3f, tep );

   // ---- kết thúc phần đầu
   fputc( 0x00, tep );
   
   // ==== bảng cho thành phần dữ liệu ảnh
   // ---- giữ địa chỉ đầu bảng thành phần
   unsigned long long diaChiDauBangThanhPhan = ftell( tep );
   // ---- lưu bàng rỗng
   luuBangDuLieuAnh( tep, beRong, beCao, 3, kieuDuLieu );
   unsigned long long *bangThanhPhan = malloc( (beCao >> 1) + 1 );  // (bề cao / 16) * 8 + 1
   
   // ---- bề dài dệm
   unsigned int beDaiDem = (beRong << kieuDuLieu)*3 << 4; // nhân 3 cho 3 kênh, 16 hàng
   // ---- tạo đệm để lọc dữ liệu
   unsigned char *dem = malloc( beDaiDem );
   unsigned char *demLoc = malloc( beDaiDem );
   unsigned char *demNenZIP = malloc( beDaiDem << 1);  // nhân 2 cho an toàn
   unsigned short soThanhPhan = 0;
   
   // ---- lưu dữ liệu cho thành phần ảnh
   unsigned short soHang = 0;
   while( soHang < beCao ) {
      
      // ---- tính số lượng hàng còn
      unsigned short soLuongHangCon = beCao - soHang;
      if( soLuongHangCon > 16 )
         soLuongHangCon = 16;
      
      bangThanhPhan[soThanhPhan] = ftell( tep );
      soThanhPhan++;

      // ---- luư số hàng
      fputc( soHang & 0xff, tep );
      fputc( (soHang >> 8), tep );
      fputc( (soHang >> 16), tep );
      fputc( (soHang >> 24), tep );
      
      // ---- dữ liệu kênh
      unsigned int diaChiKenhBatDau = beRong*(beCao - soHang - 1);
      unsigned int diaChiDem = 0;
      
      // ---- gồm dữ liệu từ các hàng
      unsigned char soHangTrongThanhPhan = 0;
      
      while( soHangTrongThanhPhan < soLuongHangCon ) {
         
         // ---- tùy kiểu dữ liệu trong ảnh
         if( kieuDuLieu == kKIEU_FLOAT ) {
            // ---- chép kênh xanh
            chepDuLieuKenhFloat( &(dem[diaChiDem]), &(anh->kenhXanh[diaChiKenhBatDau]), beRong );
            // ---- chép kênh lục
            diaChiDem += beRong << kieuDuLieu;
            chepDuLieuKenhFloat( &(dem[diaChiDem]), &(anh->kenhLuc[diaChiKenhBatDau]), beRong );
            // ---- chép kênh đỏ
            diaChiDem += beRong << kieuDuLieu;
            chepDuLieuKenhFloat( &(dem[diaChiDem]), &(anh->kenhDo[diaChiKenhBatDau]), beRong );
            // ---- tiếp theo
            diaChiDem += beRong << kieuDuLieu;
            diaChiKenhBatDau -= beRong;
         }
         else {  // kKIEU_HALF
            // ---- chép kênh xanh
            chepDuLieuKenhHalf( &(dem[diaChiDem]), &(anh->kenhXanh[diaChiKenhBatDau]), beRong );
            // ---- chép kênh lục
            diaChiDem += beRong << kieuDuLieu;
            chepDuLieuKenhHalf( &(dem[diaChiDem]), &(anh->kenhLuc[diaChiKenhBatDau]), beRong );
            // ---- chép kênh đỏ
            diaChiDem += beRong << kieuDuLieu;
            chepDuLieuKenhHalf( &(dem[diaChiDem]), &(anh->kenhDo[diaChiKenhBatDau]), beRong );
            // ---- tiếp theo
            diaChiDem += beRong << kieuDuLieu;
            diaChiKenhBatDau -= beRong;
         }
         
         soHangTrongThanhPhan++;
      }

      locDuLieuTrongDem( dem, beDaiDem, demLoc);
      unsigned int beDaiDuLieuNen = nenZIP( demLoc, beDaiDem, demNenZIP, beDaiDem << 1 );
     
      fputc( beDaiDuLieuNen & 0xff, tep );
      fputc( (beDaiDuLieuNen >> 8), tep );
      fputc( (beDaiDuLieuNen >> 16), tep );
      fputc( (beDaiDuLieuNen >> 24), tep );
      
      // ---- lưu dữ liệu nén
      unsigned int diaChi = 0;
      while( diaChi < beDaiDuLieuNen ) {
         fputc( demNenZIP[diaChi], tep );
         diaChi++;
      }

      soHang+= 16;
   }
   
   // ---- lưu bảng thành phân
   fseek( tep, diaChiDauBangThanhPhan, SEEK_SET );
   soHang = 0;
   unsigned short soLuongThanhPhan = soThanhPhan;
   soThanhPhan = 0;
   while( soThanhPhan < soLuongThanhPhan ) {
      unsigned long long diaChiThanhPhan = bangThanhPhan[soThanhPhan];
      fputc( diaChiThanhPhan & 0xff, tep );
      fputc( (diaChiThanhPhan >> 8), tep );
      fputc( (diaChiThanhPhan >> 16), tep );
      fputc( (diaChiThanhPhan >> 24), tep );
      fputc( (diaChiThanhPhan >> 32), tep );
      fputc( (diaChiThanhPhan >> 40), tep );
      fputc( (diaChiThanhPhan >> 48), tep );
      fputc( (diaChiThanhPhan >> 56), tep );
      soThanhPhan++;
   }
 
   // ---- thả trí nhớ
   free( dem );
   free( demLoc );
   free( demNenZIP );
   // ---- đóng tệp
   fclose( tep );
}


void luuThongTinKenh_EXR( FILE *tep, unsigned char *danhSachKenh, unsigned char soLuongKenh, unsigned char kieuDuLieu ) {

   fprintf( tep, "channels" );
   fputc( 0x00, tep );
   fprintf( tep, "chlist" );
   fputc( 0x00, tep );
   unsigned char beDaiDuLieuKenh = soLuongKenh*18 + 1;
   fputc( beDaiDuLieuKenh, tep );   // bề dài cho n kênh, tên các kênh dài một chữ cái ASCII
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );

   // ---- thông tin cho các kênh
   unsigned char soKenh = 0;
   while( soKenh < soLuongKenh ) {
      fputc( danhSachKenh[soKenh], tep );
      fputc( 0x00, tep );
      
      fputc( kieuDuLieu, tep );  // kiểu dữ liệu 0x02 nghỉa là float, 0x01 là half
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      
      fputc( 0x00, tep );   // chỉ xài cho phương pháp nén B44, ở đây không xài
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      
      fputc( 0x01, tep );  // nhịp x
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      
      fputc( 0x01, tep );  // nhịp y
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      fputc( 0x00, tep );

      soKenh++;
   }
   
   // ---- kết thúc danh sách kênh
   fputc( 0x00, tep );
}

void luuThongTinCuaSoDuLieu( FILE *tep, unsigned int beRong, unsigned int beCao ) {
   beRong--;  // số cột cuối
   beCao--;   // số hàng cuối
   fprintf( tep, "dataWindow" );
   fputc( 0x00, tep );
   fprintf( tep, "box2i" );
   fputc( 0x00, tep );
   fputc( 16, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   
   // ---- góc x
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   // ---- góc y
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   // ---- cột cuối
   fputc( beRong & 0xff, tep );
   fputc( (beRong >> 8) & 0xff, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   // ---- hàng cuối
   fputc( beCao & 0xff, tep );
   fputc( (beCao >> 8), tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
}

void luuThongTinCuaSoChieu( FILE *tep, unsigned int beRong, unsigned int beCao ) {
   beRong--;  // số cột cuối
   beCao--;   // số hàng cuối
   fprintf( tep, "displayWindow" );
   fputc( 0x00, tep );
   fprintf( tep, "box2i" );
   fputc( 0x00, tep );
   fputc( 16, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   
   // ---- góc x
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   // ---- góc y
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   // ---- cột cuối
   fputc( beRong & 0xff, tep );
   fputc( (beRong >> 8) & 0xff, tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
   // ---- hàng cuối
   fputc( beCao & 0xff, tep );
   fputc( (beCao >> 8), tep );
   fputc( 0x00, tep );
   fputc( 0x00, tep );
}

void luuThoiGianKetXuat( FILE *tep, unsigned short thoiGianKetXuat ) {

   // ---- tỉ số cạnh điểm ảnh
   fprintf( tep, "RenderTime" );
   fputc( 0x00, tep );
   fprintf( tep, "string" );
   fputc( 0x00, tep );

   // ---- phút
   unsigned short phut = thoiGianKetXuat/60;
   unsigned short giay = thoiGianKetXuat - phut*60;
   if( phut < 60 ) {
      fputc( 0x08, tep );   // bề dài dữ liệu
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      fprintf( tep, "%02d", phut );   // đổi thành thập phận
   }
   else {
      // ---- giờ
     unsigned short gio = phut/60;
      phut -= gio*60;
      fputc( 0x0b, tep );   // bề dài dữ liệu
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      fputc( 0x00, tep );
      gio = phut/60;
      fprintf( tep, "%02d:%02d", gio, phut );   // đổi thành thập phận
   }

   // ---- giây
   fprintf( tep, ":%02d", giay );  // đổi thành thập phận
   fputc( '.', tep );
   fputc( '0', tep );
   fputc( '0', tep );
}

void luuBangDuLieuAnh( FILE *tep, unsigned short beRong, unsigned short beCao, unsigned char soLuongKenh, unsigned char kieuDuLieu ) {
   
   // ---- tính địa chỉ cho thành phần đầu sau bảng  
   unsigned long long diaChiThanhPhan = ftell( tep ) + (beCao << 3);
   unsigned long long soLuongByteMotHangAnh = (beRong*soLuongKenh << kieuDuLieu) + 8;  // mỗi kênh là kiếu float cho nên nhân 4

   unsigned short soHang = 0;
   while( soHang < beCao ) {
      fputc( diaChiThanhPhan & 0xff, tep );
      fputc( (diaChiThanhPhan >> 8), tep );
      fputc( (diaChiThanhPhan >> 16), tep );
      fputc( (diaChiThanhPhan >> 24), tep );
      fputc( (diaChiThanhPhan >> 32), tep );
      fputc( (diaChiThanhPhan >> 40), tep );
      fputc( (diaChiThanhPhan >> 48), tep );
      fputc( (diaChiThanhPhan >> 56), tep );
      diaChiThanhPhan += soLuongByteMotHangAnh;
      soHang++;
   }

}

void chepDuLieuKenhFloat( unsigned char *dem, const float *kenh, unsigned short beRong ) {

   unsigned short soCot = 0;
   unsigned int diaChiDem = 0;
   while( soCot < beRong ) {

      union {
         float f;
         unsigned int i;
      } ui;

      ui.f = kenh[soCot];
      dem[diaChiDem] = ui.i & 0xff;
      dem[diaChiDem + 1] = (ui.i >> 8) & 0xff;
      dem[diaChiDem + 2] = (ui.i >> 16) & 0xff;
      dem[diaChiDem + 3] = (ui.i >> 24) & 0xff;
      diaChiDem += 4;
      soCot++;
   }

}

void chepDuLieuKenhHalf( unsigned char *dem, const float *kenh, unsigned short beRong ) {
   
   unsigned short soCot = 0;
   unsigned int diaChiDem = 0;
   while( soCot < beRong ) {

      unsigned short h = doiFloatSangHalf( kenh[soCot] );
      dem[diaChiDem] = h & 0xff;
      dem[diaChiDem + 1] = (h >> 8) & 0xff;
      diaChiDem += 2;
      soCot++;
   }

}

// ---- rất đơn giản, cho tốc đ`ộ nhanh và biết nguồn dữ liệu, KHÔNG THEO TOÀN CHUẨN EXR cho đổi float
// giá trị nào âm, NaN, ∞ đặt = 0,0
unsigned short doiFloatSangHalf( float soFloat ) {
 
   union {
      float f;
      unsigned int i;
   } ui;

   ui.f = soFloat;

   unsigned short soHalf;
   if( ui.i & 0x80000000 )   // âm
      soHalf = 0x0000;
   else if( (ui.f == 0.0f) || (ui.f == -0.0f) ) { // số không
      soHalf = 0x0000;
   }
   else {
      int muFloat = ui.i & 0x7f800000;

      if( muFloat == 0x7f800000 )  // NaN
         soHalf = 0x0000;
      else {
         char muKiemTra = (muFloat >> 23) - 127;

         if( muKiemTra < -14 ) // qúa nhỏ
            soHalf = 0x0000;
         else if( muKiemTra > 15 )
            soHalf = 0x7c00;  //+∞
         else {  // bình thường
            unsigned short muHalf = (((muFloat & 0x7f800000) >> 23) - 112) << 10;
            unsigned int dinhTriFloat = ui.i & 0x007fffff;
            unsigned short dinhTriHalf = ((dinhTriFloat + 0x00000fff + ((dinhTriFloat >> 13) & 1)) >> 13);
            soHalf = muHalf + dinhTriHalf;
         }
      }
   }

   return soHalf;
}

// ---- Từ thư viện OpenEXR
void locDuLieuTrongDem(unsigned char *dem, unsigned int beDai, unsigned char *demLoc ) {

   {
      unsigned char *t1 = demLoc;
      unsigned char *t2 = demLoc + (beDai + 1) / 2;
      char *dau = (char *)dem;  // đầu đệm cần lọc
      char *ketThuc = dau + beDai;
      unsigned char xong = kSAI;
      
      while( !xong )
      {
         if (dau < ketThuc)
            *(t1++) = *(dau++);
         else
            xong = kSAI;
         
         if (dau < ketThuc)
            *(t2++) = *(dau++);
         else
            xong = kDUNG;
      }
   }
   
   // trừ
   {
      unsigned char *t = (unsigned char *)demLoc + 1;
      unsigned char *ketThuc = (unsigned char *)demLoc + beDai;
      int p = t[-1];
      
      while (t < ketThuc) {
         
         int d = (int)(t[0]) - p + 128;
         p = t[0];
         t[0] = d;
         ++t;
      }
   }

}

// ---- Từ thư viện OpenEXR
unsigned int nenZIP(unsigned char *dem, int beDaiDem, unsigned char *demNen, int beDaiDemNen ) {
   
   int err;
   z_stream dongNen; // dòng nén
   
   dongNen.zalloc = Z_NULL;
   dongNen.zfree = Z_NULL;
   dongNen.opaque = Z_NULL;
   
   
   // ---- kiểm tra sai có lầm khởi đầu
   err = deflateInit(&dongNen, Z_DEFAULT_COMPRESSION);
   
   if( err != Z_OK )
      printf( "nenZip: Vấn đề khởi đầu nén %d (%x) dongNen.avail_in %d", err, err, dongNen.avail_in );
   
   // ---- cho dữ liệu cần nén
   dongNen.next_in = dem;
   dongNen.avail_in = beDaiDem;
   
   // ---- xem nếu đệm có thể chứa dữ liệu nén  
   unsigned int beDaiDuDoan = (unsigned int)deflateBound(&dongNen, beDaiDem );
   if( beDaiDuDoan > beDaiDemNen )
      printf( "nenZIP: dự đoán beDaiDuDoan %d > đệm chứa beDaiDemNen %d", beDaiDuDoan, beDaiDemNen );
   
   // ---- cho đệm cho chứa dữ liệu nén
   dongNen.next_out  = demNen;
   dongNen.avail_out = beDaiDemNen;  // bề dải đệm chứa dữ liệu nén
   err = deflate(&dongNen, Z_FINISH);
   
   if( err != Z_STREAM_END ) {
      if( err == Z_OK) {
         printf( "nenZIP: Z_OK d_stream.avail_out %d d_stream.total_out %lu",
               dongNen.avail_out, dongNen.total_out );
      }
      else
         printf( "nenZIP: sai lầm hết dữ liệu sớm hơn ý định, deflate %d (%x) d_stream.avail_out %d d_stream.total_out %lu",
               err, err, dongNen.avail_in, dongNen.total_in );
   }
   
   err = deflateEnd( &dongNen );
   if( err != Z_OK )
      printf( "nenZIP: Sai lầm deflateEnd %d (%x) dongNen.avail_out %d", err, err, dongNen.avail_out );

   return dongNen.total_out;
}

#pragma mark ---- Bezier
Vecto tinhViTriBezier3C( Bezier *bezier, float thamSo ) {
   
   // ---- mức 0, cần tính 3 điểm
   Vecto muc0[3];
   muc0[0].x = (1.0f - thamSo)*bezier->diemQuanTri[0].x + thamSo*bezier->diemQuanTri[1].x;
   muc0[0].y = (1.0f - thamSo)*bezier->diemQuanTri[0].y + thamSo*bezier->diemQuanTri[1].y;
   muc0[0].z = (1.0f - thamSo)*bezier->diemQuanTri[0].z + thamSo*bezier->diemQuanTri[1].z;
   muc0[1].x = (1.0f - thamSo)*bezier->diemQuanTri[1].x + thamSo*bezier->diemQuanTri[2].x;
   muc0[1].y = (1.0f - thamSo)*bezier->diemQuanTri[1].y + thamSo*bezier->diemQuanTri[2].y;
   muc0[1].z = (1.0f - thamSo)*bezier->diemQuanTri[1].z + thamSo*bezier->diemQuanTri[2].z;
   muc0[2].x = (1.0f - thamSo)*bezier->diemQuanTri[2].x + thamSo*bezier->diemQuanTri[3].x;
   muc0[2].y = (1.0f - thamSo)*bezier->diemQuanTri[2].y + thamSo*bezier->diemQuanTri[3].y;
   muc0[2].z = (1.0f - thamSo)*bezier->diemQuanTri[2].z + thamSo*bezier->diemQuanTri[3].z;
   
   // ---- mức 1, cần tính 2 điểm
   Vecto muc1[2];
   muc1[0].x = (1.0f - thamSo)*muc0[0].x + thamSo*muc0[1].x;
   muc1[0].y = (1.0f - thamSo)*muc0[0].y + thamSo*muc0[1].y;
   muc1[0].z = (1.0f - thamSo)*muc0[0].z + thamSo*muc0[1].z;
   muc1[1].x = (1.0f - thamSo)*muc0[1].x + thamSo*muc0[2].x;
   muc1[1].y = (1.0f - thamSo)*muc0[1].y + thamSo*muc0[2].y;
   muc1[1].z = (1.0f - thamSo)*muc0[1].z + thamSo*muc0[2].z;
   
   Vecto viTri;
   viTri.x = (1.0f - thamSo)*muc1[0].x + thamSo*muc1[1].x;
   viTri.y = (1.0f - thamSo)*muc1[0].y + thamSo*muc1[1].y;
   viTri.z = (1.0f - thamSo)*muc1[0].z + thamSo*muc1[1].z;

   return viTri;
}

// ds(t)dt = -x0*3*(1-t)^2 + x1*3*[(1-t)^2 - 2*t(1-t)] + x2*3[2*t(1-t) - t^2] + x3*3*t^2
// ds(t)dt = 3*(1-t) * [-x0*(1-t) + x1*(1-3t)]    +    3*t * [x2*(2-3t) + x3*t]
Vecto tinhVanTocBezier3C( Bezier *bezier, float thamSo ) {
   
   // ---- mức 0, cần tính 3 điểm
   Vecto muc0[2];
   muc0[0].x = -(1.0f - thamSo)*bezier->diemQuanTri[0].x + (1.0f - 3.0f*thamSo)*bezier->diemQuanTri[1].x;
   muc0[0].y = -(1.0f - thamSo)*bezier->diemQuanTri[0].y + (1.0f - 3.0f*thamSo)*bezier->diemQuanTri[1].y;
   muc0[0].z = -(1.0f - thamSo)*bezier->diemQuanTri[0].z + (1.0f - 3.0f*thamSo)*bezier->diemQuanTri[1].z;
   muc0[1].x = (2.0f - 3.0f*thamSo)*bezier->diemQuanTri[2].x + thamSo*bezier->diemQuanTri[3].x;
   muc0[1].y = (2.0f - 3.0f*thamSo)*bezier->diemQuanTri[2].y + thamSo*bezier->diemQuanTri[3].y;
   muc0[1].z = (2.0f - 3.0f*thamSo)*bezier->diemQuanTri[2].z + thamSo*bezier->diemQuanTri[3].z;

   Vecto vanToc;
   vanToc.x = (1.0f - thamSo)*muc0[0].x + thamSo*muc0[1].x;
   vanToc.y = (1.0f - thamSo)*muc0[0].y + thamSo*muc0[1].y;
   vanToc.z = (1.0f - thamSo)*muc0[0].z + thamSo*muc0[1].z;

   vanToc.x *= 3.0f;
   vanToc.y *= 3.0f;
   vanToc.z *= 3.0f;
   
   return vanToc;
}

#pragma mark ---- Họa Tiết Không
HoaTietKhong datHoaTietKhong( Mau *mau ) {
   HoaTietKhong hoaTietKhong;
   hoaTietKhong.mau.d = mau->d;
   hoaTietKhong.mau.l = mau->l;
   hoaTietKhong.mau.x = mau->x;
   hoaTietKhong.mau.dd = mau->dd;
   hoaTietKhong.mau.p = mau->p;

   return hoaTietKhong;
}

#pragma mark ---- Họa Tiết Màu Dị Hướng
HoaTietDiHuong datHoaTietDiHuong( Mau *mauRanh, Mau *mauTam ) {
   HoaTietDiHuong hoaTietDiHuong;
   hoaTietDiHuong.mauRanh.d = mauRanh->d;
   hoaTietDiHuong.mauRanh.l = mauRanh->l;
   hoaTietDiHuong.mauRanh.x = mauRanh->x;
   hoaTietDiHuong.mauRanh.dd = mauRanh->dd;
   hoaTietDiHuong.mauRanh.p = mauRanh->p;
   
   hoaTietDiHuong.mauTam.d = mauTam->d;
   hoaTietDiHuong.mauTam.l = mauTam->l;
   hoaTietDiHuong.mauTam.x = mauTam->x;
   hoaTietDiHuong.mauTam.dd = mauTam->dd;
   hoaTietDiHuong.mauTam.p = mauTam->p;
   
   return hoaTietDiHuong;
}

Mau hoaTietDiHuong( Vecto phapThuyen, Vecto huongTia, HoaTietDiHuong *hoaTietDiHuong ) {
   
   donViHoa( &huongTia );
   float tichVoHuong = phapThuyen.x*huongTia.x + phapThuyen.y*huongTia.y + phapThuyen.z*huongTia.z;

   // ---- xài giá trị tuyệt đối
   if( tichVoHuong < 0.0f )
      tichVoHuong = -tichVoHuong;
   
   tichVoHuong *= tichVoHuong;

   float nghichTichVoHuong = 1.0f - tichVoHuong;

   Mau mauDiHuong;
   mauDiHuong.d = tichVoHuong*hoaTietDiHuong->mauTam.d + nghichTichVoHuong*hoaTietDiHuong->mauRanh.d;
   mauDiHuong.l = tichVoHuong*hoaTietDiHuong->mauTam.l + nghichTichVoHuong*hoaTietDiHuong->mauRanh.l;
   mauDiHuong.x = tichVoHuong*hoaTietDiHuong->mauTam.x + nghichTichVoHuong*hoaTietDiHuong->mauRanh.x;
   mauDiHuong.dd = tichVoHuong*hoaTietDiHuong->mauTam.dd + nghichTichVoHuong*hoaTietDiHuong->mauRanh.dd;
   mauDiHuong.p = tichVoHuong*hoaTietDiHuong->mauTam.p + nghichTichVoHuong*hoaTietDiHuong->mauRanh.p;

   return mauDiHuong;
}

#pragma mark ---- Họa Tiết Ca Rô
HoaTietCaRo datHoaTietCaRo( Mau *mau0, Mau *mau1, float beRong, float beCao, float beDai ) {
   HoaTietCaRo hoaTietCaRo;
   hoaTietCaRo.mau0.d = mau0->d;
   hoaTietCaRo.mau0.l = mau0->l;
   hoaTietCaRo.mau0.x = mau0->x;
   hoaTietCaRo.mau0.dd = mau0->dd;
   hoaTietCaRo.mau0.p = mau0->p;

   hoaTietCaRo.mau1.d = mau1->d;
   hoaTietCaRo.mau1.l = mau1->l;
   hoaTietCaRo.mau1.x = mau1->x;
   hoaTietCaRo.mau1.dd = mau1->dd;
   hoaTietCaRo.mau1.p = mau1->p;
   
   hoaTietCaRo.beRong = beRong;
   hoaTietCaRo.beCao = beCao;
   hoaTietCaRo.beDai = beDai;
   
   return hoaTietCaRo;
}

Mau hoaTietCaRo( Vecto *viTri, HoaTietCaRo *hoaTietCaRo ) {
   
   int soX = ceil((viTri->x)/hoaTietCaRo->beRong);
   int soY = ceil((viTri->y)/hoaTietCaRo->beCao);
   int soZ = ceil((viTri->z)/hoaTietCaRo->beDai);

   Mau mauCaRo;

   if( soY & 0x01 ){
      if( ((soX & 0x01) && (soZ & 0x01)) || (!(soX & 0x01) && !(soZ & 0x01))  ) {
         mauCaRo.d = hoaTietCaRo->mau1.d;
         mauCaRo.l = hoaTietCaRo->mau1.l;
         mauCaRo.x = hoaTietCaRo->mau1.x;
         mauCaRo.dd = hoaTietCaRo->mau1.dd;
         mauCaRo.p = hoaTietCaRo->mau1.p;
      }
      else {
         mauCaRo.d = hoaTietCaRo->mau0.d;
         mauCaRo.l = hoaTietCaRo->mau0.l;
         mauCaRo.x = hoaTietCaRo->mau0.x;
         mauCaRo.dd = hoaTietCaRo->mau0.dd;
         mauCaRo.p = hoaTietCaRo->mau0.p;
      }
   }
   else {
      if( ((soX & 0x01) && (soZ & 0x01)) || (!(soX & 0x01) && !(soZ & 0x01))  ) {
         mauCaRo.d = hoaTietCaRo->mau0.d;
         mauCaRo.l = hoaTietCaRo->mau0.l;
         mauCaRo.x = hoaTietCaRo->mau0.x;
         mauCaRo.dd = hoaTietCaRo->mau0.dd;
         mauCaRo.p = hoaTietCaRo->mau0.p;
      }
      else {
         mauCaRo.d = hoaTietCaRo->mau1.d;
         mauCaRo.l = hoaTietCaRo->mau1.l;
         mauCaRo.x = hoaTietCaRo->mau1.x;
         mauCaRo.dd = hoaTietCaRo->mau1.dd;
         mauCaRo.p = hoaTietCaRo->mau1.p;
      }
   }
   
   return mauCaRo;
}

#pragma mark ---- Họa Tiết Vòng Tròn
HoaTietVongTron datHoaTietVongTron( Mau *mauNoi, Mau *mauNgoai, float banKinh ) {
   HoaTietVongTron hoaTietVongTron;
   hoaTietVongTron.mauNoi.d = mauNoi->d;
   hoaTietVongTron.mauNoi.l = mauNoi->l;
   hoaTietVongTron.mauNoi.x = mauNoi->x;
   hoaTietVongTron.mauNoi.dd = mauNoi->dd;
   hoaTietVongTron.mauNoi.p = mauNoi->p;

   hoaTietVongTron.mauNgoai.d = mauNgoai->d;
   hoaTietVongTron.mauNgoai.l = mauNgoai->l;
   hoaTietVongTron.mauNgoai.x = mauNgoai->x;
   hoaTietVongTron.mauNgoai.dd = mauNgoai->dd;
   hoaTietVongTron.mauNgoai.p = mauNgoai->p;
   
   hoaTietVongTron.banKinh = banKinh;
   return hoaTietVongTron;
}

Mau hoaTietVongTron( Vecto *viTri, HoaTietVongTron *hoaTietVongTron ) {

   // ---- chọn màu
   Mau mauTo;
   if( viTri->x*viTri->x + viTri->z*viTri->z < hoaTietVongTron->banKinh*hoaTietVongTron->banKinh ) {
      mauTo.d = hoaTietVongTron->mauNoi.d;
      mauTo.l = hoaTietVongTron->mauNoi.l;
      mauTo.x = hoaTietVongTron->mauNoi.x;
      mauTo.dd = hoaTietVongTron->mauNoi.dd;
      mauTo.p = hoaTietVongTron->mauNoi.p;
   }
   else {
      mauTo.d = hoaTietVongTron->mauNgoai.d;
      mauTo.l = hoaTietVongTron->mauNgoai.l;
      mauTo.x = hoaTietVongTron->mauNgoai.x;
      mauTo.dd = hoaTietVongTron->mauNgoai.dd;
      mauTo.p = hoaTietVongTron->mauNgoai.p;
   }
   
   return mauTo;
}

#pragma mark ---- Họa Tiết Óc Xoáy
HoaTietOcXoay datHoaTietOcXoay( Mau *mau0, Mau *mau1, float beRongNet, float phongTo ) {
   HoaTietOcXoay hoaTietOcXoay;
   hoaTietOcXoay.mau0.d = mau0->d;
   hoaTietOcXoay.mau0.l = mau0->l;
   hoaTietOcXoay.mau0.x = mau0->x;
   hoaTietOcXoay.mau0.dd = mau0->dd;
   hoaTietOcXoay.mau0.p = mau0->p;
   
   hoaTietOcXoay.mau1.d = mau1->d;
   hoaTietOcXoay.mau1.l = mau1->l;
   hoaTietOcXoay.mau1.x = mau1->x;
   hoaTietOcXoay.mau1.dd = mau1->dd;
   hoaTietOcXoay.mau1.p = mau1->p;
   
   hoaTietOcXoay.beRongNet = beRongNet;
   hoaTietOcXoay.phongTo = phongTo;

   return hoaTietOcXoay;
}

// cho phóng to, xài giá trị < 1.0 cho rộng hơn
Mau hoaTietOcXoay( Vecto *viTri, HoaTietOcXoay *hoaTietOcXoay ) {
   
   float banKinh = sqrtf( viTri->x*viTri->x + viTri->z*viTri->z );
   banKinh *= hoaTietOcXoay->phongTo;  // sẽ phóng nhỏ
   
   // ---- tính góc
   float goc = 0.0f;
   if( banKinh != 0.0f )
      goc = atan( viTri->z/viTri->x );
   
   // ---- giữ 0 ≤ góc ≤ 2π
   if( viTri->x < 0.0f )
      goc += 3.141592f;
   else
      if( viTri->z < 0.0f )
         goc += 6.283184f;
   
   // ---- tính vòng
   unsigned short soVong = banKinh/6.283184f;
   
   goc += (float)soVong*6.283184f;
   
   // ---- chọn màu
   Mau mauTo;
   float nuaBeRongNet = hoaTietOcXoay->beRongNet*0.5f;
   
   if( (goc < banKinh + nuaBeRongNet) && (goc > banKinh - nuaBeRongNet ) ) {
      mauTo.d = hoaTietOcXoay->mau0.d;
      mauTo.l = hoaTietOcXoay->mau0.l;
      mauTo.x = hoaTietOcXoay->mau0.x;
      mauTo.dd = hoaTietOcXoay->mau0.dd;
      mauTo.p = hoaTietOcXoay->mau0.p;
   }
   else {
      if( goc < banKinh )
         goc += 6.283184f;
      else if( goc > banKinh )
         goc -= 6.283184f;
      if( (goc < banKinh + nuaBeRongNet) && (goc > banKinh - nuaBeRongNet ) ) {
         mauTo.d = hoaTietOcXoay->mau0.d;
         mauTo.l = hoaTietOcXoay->mau0.l;
         mauTo.x = hoaTietOcXoay->mau0.x;
         mauTo.dd = hoaTietOcXoay->mau0.dd;
         mauTo.p = hoaTietOcXoay->mau0.p;
      }
      else {
         mauTo.d = hoaTietOcXoay->mau1.d;
         mauTo.l = hoaTietOcXoay->mau1.l;
         mauTo.x = hoaTietOcXoay->mau1.x;
         mauTo.dd = hoaTietOcXoay->mau1.dd;
         mauTo.p = hoaTietOcXoay->mau1.p;
      }
   }
   //   printf( "goc %5.3f  banKinh %5.3f %5.3f  soVong %d  mauTo %5.3f\n", goc, banKinh - 0.5f*beRongNet, banKinh + 0.5f*beRongNet, soVong, mauTo.d);
   
   return mauTo;
}

#pragma mark ---- Họa Tiết Gằn
HoaTietGan datHoaTietGan( Mau *mauTren, Mau *mauDuoi, float beCaoTren, float beCaoDuoi ) {
   
   HoaTietGan hoaTietGan;
   hoaTietGan.mauTren.d = mauTren->d;
   hoaTietGan.mauTren.l = mauTren->l;
   hoaTietGan.mauTren.x = mauTren->x;
   hoaTietGan.mauTren.dd = mauTren->dd;
   hoaTietGan.mauTren.p = mauTren->p;

   hoaTietGan.mauDuoi.d = mauDuoi->d;
   hoaTietGan.mauDuoi.l = mauDuoi->l;
   hoaTietGan.mauDuoi.x = mauDuoi->x;
   hoaTietGan.mauDuoi.dd = mauDuoi->dd;
   hoaTietGan.mauDuoi.p = mauDuoi->p;

   hoaTietGan.beCaoTren = beCaoTren;
   hoaTietGan.beCaoDuoi = beCaoDuoi;

   return hoaTietGan;
}

Mau hoaTietGan( float toaDo, HoaTietGan *hoaTietGan ) {
   
   // ---- tính bề cao cả hai lớp
   float beCaoHaiLop = hoaTietGan->beCaoTren + hoaTietGan->beCaoDuoi;
   // ---- tính số dư đơn vị hóa
   int so = floor(toaDo/beCaoHaiLop);
   // ---- tính số dư
   float du = (toaDo - so*beCaoHaiLop)/beCaoHaiLop;

   Mau mauTo;
   if( du < hoaTietGan->beCaoDuoi ) {
      mauTo.d = hoaTietGan->mauDuoi.d;
      mauTo.l = hoaTietGan->mauDuoi.l;
      mauTo.x = hoaTietGan->mauDuoi.x;
      mauTo.dd = hoaTietGan->mauDuoi.dd;
      mauTo.p = hoaTietGan->mauDuoi.p;
   }
   else {
      mauTo.d = hoaTietGan->mauTren.d;
      mauTo.l = hoaTietGan->mauTren.l;
      mauTo.x = hoaTietGan->mauTren.x;
      mauTo.dd = hoaTietGan->mauTren.dd;
      mauTo.p = hoaTietGan->mauTren.p;
   }

   return mauTo;
}

#pragma mark ---- Hoạ Tiết Trái Banh
// tính màu xài tọa độ kinh vĩ tuyến
Mau hoaTietTraiBanh( Vecto *viTri ) {
   
   // ---- tính bán kính
   float banKinh = sqrtf( viTri->x*viTri->x + viTri->z*viTri->z );
   
   // ---- tính kính độ
   float kinhDo = 0.0f;
   if( banKinh != 0.0f )
      kinhDo = atan( viTri->z/viTri->x );
   
   // ---- giữ 0 ≤ kinh độ ≤ 2π
   if( viTri->x < 0.0f )
      kinhDo += 3.141592f;
   else
      if( viTri->z < 0.0f )
         kinhDo += 6.283184f;
   
   // ---- vĩ độ
   float viDo = 0.0f;
   if( banKinh != 0.0f )
      viDo = atan( viTri->y/banKinh );
   
   Mau mauTo;  // 
   // 24 16 24   24 16 24
   if( kinhDo < 1.570796f ) {
      if( viDo < -0.981748f ) {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
      else if( viDo < -0.589049f ) {
         mauTo.d = 0.309803f;
         mauTo.l = 0.294118f;
         mauTo.x = 0.690196f;
      }
      else if( viDo < 0.0f ) {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
      else if( viDo < 0.589049f ) {
         mauTo.d = 0.525490f;
         mauTo.l = 0.000000f;
         mauTo.x = 0.541176f;
      }
      else if( viDo < 0.981748f ) {
         mauTo.d = 0.925490f;
         mauTo.l = 0.827451f;
         mauTo.x = 0.925490f;
      }
      else {
         mauTo.d = 0.525490f;
         mauTo.l = 0.000000f;
         mauTo.x = 0.541176f;
      }
   }
   else if( kinhDo < 3.141592f ) {
      if( viDo < -0.981748f ) {
         mauTo.d = 0.305882f;
         mauTo.l = 0.200000f;
         mauTo.x = 0.450980f;
      }
      else if( viDo < -0.589049f ) {
         mauTo.d = 0.862745f;
         mauTo.l = 0.854902f;
         mauTo.x = 0.925492f;
      }
      else if( viDo < 0.0f ) {
         mauTo.d = 0.305882f;
         mauTo.l = 0.200000f;
         mauTo.x = 0.450980f;
      }
      else if( viDo < 0.589049f ) {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
      else if( viDo < 0.981748f ) {
         mauTo.d = 0.698039f;
         mauTo.l = 0.356863f;
         mauTo.x = 0.698039f;
         mauTo.dd = 1.0f;
      }
      else {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
   }
   else if( kinhDo < 4.712388f ) {
      if( viDo < -0.981748f ) {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
      else if( viDo < -0.589049f ) {
         mauTo.d = 0.674510f;
         mauTo.l = 0.298039f;
         mauTo.x = 0.137255f;
      }
      else if( viDo < 0.0f ) {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
      else if( viDo < 0.5890494f ) {
         mauTo.d = 0.694118f;
         mauTo.l = 0.466667f;
         mauTo.x = 0.000000f;
      }
      else if( viDo < 0.981748f ) {
         mauTo.d = 1.000000f;
         mauTo.l = 0.921569f;
         mauTo.x = 0.878431f;
      }
      else {
         mauTo.d = 0.694118f;
         mauTo.l = 0.466667f;
         mauTo.x = 0.000000f;
      }   }
   else {
      if( viDo < -0.981748f ) {
         mauTo.d = 0.796078f;
         mauTo.l = 0.509804f;
         mauTo.x = 0.000000f;
      }
      else if( viDo < -0.589049f ) {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
      else if( viDo < 0.0f ) {
         mauTo.d = 0.796078f;
         mauTo.l = 0.509804f;
         mauTo.x = 0.000000f;
      }
      else if( viDo < 0.589049f ) {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
      else if( viDo < 0.981748f ) {
         mauTo.d = 0.805882f;
         mauTo.l = 0.352941f;
         mauTo.x = 0.000000f;
      }
      else {
         mauTo.d = 1.0f;
         mauTo.l = 1.0f;
         mauTo.x = 1.0f;
      }
   }
   
   mauTo.dd = 1.0f;
   mauTo.p = 0.05f;

   return mauTo;
}

#pragma mark ---- Họa Tiết Chấm Bi
HoaTietChamBi datHoaTietChamBi( Mau *mauNen, Mau *mauThap, Mau *mauCao ) {
   HoaTietChamBi hoaTietChamBi;
   hoaTietChamBi.mauNen.d = mauNen->d;
   hoaTietChamBi.mauNen.l = mauNen->l;
   hoaTietChamBi.mauNen.x = mauNen->x;
   hoaTietChamBi.mauNen.dd = mauNen->dd;
   hoaTietChamBi.mauNen.p = mauNen->p;
   
   Mau phamViMau;
   phamViMau.d = mauCao->d - mauThap->d;
   phamViMau.l = mauCao->l - mauThap->l;
   phamViMau.x = mauCao->x - mauThap->x;
   phamViMau.dd = mauCao->dd - mauThap->dd;
   phamViMau.p = mauCao->p - mauThap->p;

   unsigned char soCham = 0;
   while( soCham < 48 ) {
      hoaTietChamBi.mangMau[soCham].d = phamViMau.d*rand()/kSO_NGUYEN_TOI_DA + mauThap->d;
      hoaTietChamBi.mangMau[soCham].l = phamViMau.l*rand()/kSO_NGUYEN_TOI_DA + mauThap->l;
      hoaTietChamBi.mangMau[soCham].x = phamViMau.x*rand()/kSO_NGUYEN_TOI_DA + mauThap->x;
      hoaTietChamBi.mangMau[soCham].dd = phamViMau.dd*rand()/kSO_NGUYEN_TOI_DA + mauThap->dd;
      hoaTietChamBi.mangMau[soCham].p = phamViMau.p*rand()/kSO_NGUYEN_TOI_DA + mauThap->p;
//      printf( "%5.3f %5.3f %5.3f\n", hoaTietChamBi.mangMau[soCham].d, hoaTietChamBi.mangMau[soCham].l, hoaTietChamBi.mangMau[soCham].x );
      hoaTietChamBi.mangBanKinh[soCham] = 1.0f - 0.03f*rand()/kSO_NGUYEN_TOI_DA;

      hoaTietChamBi.mangVecto[soCham].x = 1.0f - rand()/1.07e9f;   // cho có phạm vi -1,0f đến 1,0f
      hoaTietChamBi.mangVecto[soCham].y = 1.0f - rand()/1.07e9f;
      hoaTietChamBi.mangVecto[soCham].z = 1.0f - rand()/1.07e9f;
      donViHoa( &(hoaTietChamBi.mangVecto[soCham]) );

      soCham++;
   }
   return hoaTietChamBi;
}

Mau hoaTietChamBi( Vecto *viTri, HoaTietChamBi *hoaTietChamBi ) {
   
   donViHoa( viTri );
   // ---- tìm gần chấm bi nào
   unsigned char coMau = kSAI;
   unsigned char soCham = 0;
   while( soCham < 48 && !coMau ) {
      
      float tichVoHuong = viTri->x*hoaTietChamBi->mangVecto[soCham].x + viTri->y*hoaTietChamBi->mangVecto[soCham].y + viTri->z*hoaTietChamBi->mangVecto[soCham].z;
      if( tichVoHuong > hoaTietChamBi->mangBanKinh[soCham] )
         coMau = kDUNG;

      soCham++;
   }

   Mau mau;
   if( coMau ) {   // màu chấm bi
      soCham--;
      mau.d = hoaTietChamBi->mangMau[soCham].d;
      mau.l = hoaTietChamBi->mangMau[soCham].l;
      mau.x = hoaTietChamBi->mangMau[soCham].x;
      mau.dd = hoaTietChamBi->mangMau[soCham].dd;
      mau.p = hoaTietChamBi->mangMau[soCham].p;
  }
   else {   // màu nền
      mau.d = hoaTietChamBi->mauNen.d;
      mau.l = hoaTietChamBi->mauNen.l;
      mau.x = hoaTietChamBi->mauNen.x;
      mau.dd = hoaTietChamBi->mauNen.dd;
      mau.p = hoaTietChamBi->mauNen.p;
   }

   return mau;
}

#pragma mark ---- Họa Tiết Ngôi Sao Cầu
// Chọn tâm ngẫu nhiên xong rồi 
// bán kính nội và ngoại là góc (rag trên mặt hình cầu)
HoaTietNgoiSaoCau datHoaTietNgoiSaoCau( Mau *mauNen, Mau *mauThap, Mau *mauCao, float banKinhNoi, float banKinhNgoai, float chenhLech,  unsigned char soLuongNan ) {

   HoaTietNgoiSaoCau hoaTietNgoiSaoCau;
   hoaTietNgoiSaoCau.mauNen.d = mauNen->d;
   hoaTietNgoiSaoCau.mauNen.l = mauNen->l;
   hoaTietNgoiSaoCau.mauNen.x = mauNen->x;
   hoaTietNgoiSaoCau.mauNen.dd = mauNen->dd;
   hoaTietNgoiSaoCau.mauNen.p = mauNen->p;
   
   // ---- coi trừng số lượng nan ≥ 2
   if( soLuongNan < 2 )
      soLuongNan = 2;
   
   // ---- hạn chế chênh lệch
   if( chenhLech < 0.0f )
      chenhLech = 0.0f;
   else if( chenhLech > 10.0f )
      chenhLech = 10.0f;
   
   // --- coi trừng bán kính nội > bán kính ngoại
   if( banKinhNgoai < banKinhNoi ) {
      float so = banKinhNoi;
      banKinhNoi = banKinhNgoai;
      banKinhNgoai = so;
   }

   // ---- góc giữa nan
   hoaTietNgoiSaoCau.goc = 6.283184f/(float)soLuongNan;   // 2π/soLuongNan
   
   // --- góc từ nan đến điểm giữa nan
   hoaTietNgoiSaoCau.nuaGoc = hoaTietNgoiSaoCau.goc*0.5f;   // π/soLuongNan

   // ---- tìm bán kính và góc cho đường ranh ngôi sao
   //       đơn vị hóa bán kính nội và xài bán kính ngoại = 1,0 cho được xài một bán kính rành
   tinhBanKinhVaGocRanh( &(hoaTietNgoiSaoCau.banKinhRanh), &(hoaTietNgoiSaoCau.gocRanh), banKinhNoi/banKinhNgoai, 1.0f, hoaTietNgoiSaoCau.nuaGoc );

   Mau phamViMau;
   phamViMau.d = mauCao->d - mauThap->d;
   phamViMau.l = mauCao->l - mauThap->l;
   phamViMau.x = mauCao->x - mauThap->x;
   phamViMau.dd = mauCao->dd - mauThap->dd;
   phamViMau.p = mauCao->p - mauThap->p;
   
   unsigned char soNgoiSao = 0;
   while( soNgoiSao < 48 ) {
      // ---- chọn màu ngẫu nhiên
      hoaTietNgoiSaoCau.mangMau[soNgoiSao].d = phamViMau.d*rand()/kSO_NGUYEN_TOI_DA + mauThap->d;
      hoaTietNgoiSaoCau.mangMau[soNgoiSao].l = phamViMau.l*rand()/kSO_NGUYEN_TOI_DA + mauThap->l;
      hoaTietNgoiSaoCau.mangMau[soNgoiSao].x = phamViMau.x*rand()/kSO_NGUYEN_TOI_DA + mauThap->x;
      hoaTietNgoiSaoCau.mangMau[soNgoiSao].dd = phamViMau.dd*rand()/kSO_NGUYEN_TOI_DA + mauThap->dd;
      hoaTietNgoiSaoCau.mangMau[soNgoiSao].p = phamViMau.p*rand()/kSO_NGUYEN_TOI_DA + mauThap->p;

      float phongTo = 1.0f + (rand()/kSO_NGUYEN_TOI_DA)*chenhLech;
      hoaTietNgoiSaoCau.mangBanKinhNgoai[soNgoiSao] = banKinhNgoai*phongTo;
      hoaTietNgoiSaoCau.mangBanKinhNoi[soNgoiSao] = banKinhNoi*phongTo;
      hoaTietNgoiSaoCau.mangBanKinhRanh[soNgoiSao] = hoaTietNgoiSaoCau.banKinhRanh*hoaTietNgoiSaoCau.mangBanKinhNgoai[soNgoiSao];

      // ---- chọn tâm trên mặt hình cầu ngẫu nhiên,
      hoaTietNgoiSaoCau.mangVecto[soNgoiSao].x = 1.0f - rand()/1.07e9f;   // cho có phạm vi -1,0f đến 1,0f
      hoaTietNgoiSaoCau.mangVecto[soNgoiSao].y = 1.0f - rand()/1.07e9f;
      hoaTietNgoiSaoCau.mangVecto[soNgoiSao].z = 1.0f - rand()/1.07e9f;
      donViHoa( &(hoaTietNgoiSaoCau.mangVecto[soNgoiSao]) );
      
      // ---- tính hướng cho ngôi sao
      // một hướng ngẫu nhiên
      Vecto vectoHuong;
      vectoHuong.x = 1.0f - rand()/1.07e9f;   // cho có phạm vi -1,0f đến 1,0f
      vectoHuong.y = 1.0f - rand()/1.07e9f;
      vectoHuong.z = 1.0f - rand()/1.07e9f;
      donViHoa( &(vectoHuong) );

      hoaTietNgoiSaoCau.mangHuong0[soNgoiSao] = tichCoHuong( &vectoHuong, &(hoaTietNgoiSaoCau.mangVecto[soNgoiSao]) );
      donViHoa( &(hoaTietNgoiSaoCau.mangHuong0[soNgoiSao]) );
      hoaTietNgoiSaoCau.mangHuong1[soNgoiSao] = tichCoHuong( &(hoaTietNgoiSaoCau.mangHuong0[soNgoiSao]), &(hoaTietNgoiSaoCau.mangVecto[soNgoiSao]) );
      donViHoa( &(hoaTietNgoiSaoCau.mangHuong1[soNgoiSao]) );

      soNgoiSao++;
   }

   return hoaTietNgoiSaoCau;
}

Mau hoaTietNgoiSaoCau( Vecto *viTri, HoaTietNgoiSaoCau *hoaTietNgoiSaoCau ) {
   
   donViHoa( viTri );
   // ---- tìm gần chấm bi nào
   unsigned char coMau = kSAI;
   unsigned char soNgoiSao = 0;
   while( (soNgoiSao < 48) && !coMau ) {
      coMau = kSAI;
      // ---- tính bán kính
      float tichVoHuong = viTri->x*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].x + viTri->y*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].y + viTri->z*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].z;
      // ---- tính góc từ tâm ngôi sao
      float gocTuTam = acosf( tichVoHuong );
//      printf( " gocTuTam %5.3f", gocTuTam );
//      printf( "%5.3f %5.3f %5.3f  gocTuTam %5.3f   tichVoHuong %5.3f   BKNgoai %5.3f  BKNoi %5.3f\n", viTri->x, viTri->y, viTri->z, gocTuTam, tichVoHuong, hoaTietNgoiSaoCau->mangBanKinhNgoai[soNgoiSao], hoaTietNgoiSaoCau->mangBanKinhNoi[soNgoiSao] );
      // ---- trong bán kính nội, phải tô màu ngôi sao
      if( gocTuTam < hoaTietNgoiSaoCau->mangBanKinhNoi[soNgoiSao] )
         coMau = kDUNG;
      else if( gocTuTam > hoaTietNgoiSaoCau->mangBanKinhNgoai[soNgoiSao] )
         coMau = kSAI;
      else {
         // ==== tính vectơ vuông với mặt tâm
         // ---- tính tích vô hướng điểm với vectơ tâm
         float tichVoHuongVoiTam = viTri->x*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].x
                              + viTri->y*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].y
                              + viTri->z*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].z;
//         printf( "tichVoHuongVoiTam %5.3f  ", tichVoHuongVoiTam );
         float doLonDenTam = sqrtf( hoaTietNgoiSaoCau->mangVecto[soNgoiSao].x*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].x
                                   + hoaTietNgoiSaoCau->mangVecto[soNgoiSao].y*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].y
                                   + hoaTietNgoiSaoCau->mangVecto[soNgoiSao].z*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].z );  // cho hình cầu có thể xài bán kính nhưng vật thể khác không có bán kính
         // ----
         Vecto vectoVuongMat;
         vectoVuongMat.x = viTri->x - tichVoHuongVoiTam*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].x/doLonDenTam;
         vectoVuongMat.y = viTri->y - tichVoHuongVoiTam*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].y/doLonDenTam;
         vectoVuongMat.z = viTri->z - tichVoHuongVoiTam*hoaTietNgoiSaoCau->mangVecto[soNgoiSao].z/doLonDenTam;

         // ---- đơn vị hóa cho tìm góc tương đối với tọa độ ngôi sao
         donViHoa( &vectoVuongMat );

         // ==== tính góc tương đối toạ độ tâm
         float goc0 = acos( vectoVuongMat.x*hoaTietNgoiSaoCau->mangHuong0[soNgoiSao].x + vectoVuongMat.y*hoaTietNgoiSaoCau->mangHuong0[soNgoiSao].y + vectoVuongMat.z*hoaTietNgoiSaoCau->mangHuong0[soNgoiSao].z );
         float goc1 = acos( vectoVuongMat.x*hoaTietNgoiSaoCau->mangHuong1[soNgoiSao].x + vectoVuongMat.y*hoaTietNgoiSaoCau->mangHuong1[soNgoiSao].y + vectoVuongMat.z*hoaTietNgoiSaoCau->mangHuong1[soNgoiSao].z );

         // ---- giữ 0 ≤ kinh độ ≤ 2π
//         if( goc1 > 1.5707963f )
//            6.283184f - goc0; // <-----
         
         // ---- tìm ở đâu giữa nan
         float gocSao = goc0 - hoaTietNgoiSaoCau->goc*floorf( goc0/hoaTietNgoiSaoCau->goc );

         // ---- đơn vị hóa trong phạm vị giữa hai nan
         //            0,0       0,5       1,0 <--- góc đơn vị hóa
         //             +         |         +
         //             | \               / |
         //             |    \    |    /    |
         //             |       \   /       |
         //             +---------+---------+   ---> góc
         //           nan                  nan

         // ---- góc sao đơn vị hóa
         float gocSaoDonViHoa = gocSao/hoaTietNgoiSaoCau->goc;

         // ---- xem cần lật gócĐơnVịHóa
         if( gocSaoDonViHoa > 0.5f )
             gocSaoDonViHoa = 1.0f - gocSaoDonViHoa;
         
         // ---- tính cách (từ tâm) của ranh ngôi sao cùng góc với điểm trúng trên mặt hình cầu
         float cachRanhNgoiSao = hoaTietNgoiSaoCau->mangBanKinhRanh[soNgoiSao]/cosf( gocSaoDonViHoa*hoaTietNgoiSaoCau->goc - hoaTietNgoiSaoCau->gocRanh );
         // ---- nếu ranh ngôi sao xa hơn, tô điểm màu của ngôi sao
         if( cachRanhNgoiSao > gocTuTam )
               coMau = kDUNG;
      }
      soNgoiSao++;
   }

   // ==== tô màu
   Mau mau;
   if( coMau ) {   // có màu ngôi sao
      soNgoiSao--;
      mau.d = hoaTietNgoiSaoCau->mangMau[soNgoiSao].d;
      mau.l = hoaTietNgoiSaoCau->mangMau[soNgoiSao].l;
      mau.x = hoaTietNgoiSaoCau->mangMau[soNgoiSao].x;
      mau.dd = hoaTietNgoiSaoCau->mangMau[soNgoiSao].dd;
      mau.p = hoaTietNgoiSaoCau->mangMau[soNgoiSao].p;
   }
   else {   // màu nền
      mau.d = hoaTietNgoiSaoCau->mauNen.d;
      mau.l = hoaTietNgoiSaoCau->mauNen.l;
      mau.x = hoaTietNgoiSaoCau->mauNen.x;
      mau.dd = hoaTietNgoiSaoCau->mauNen.dd;
      mau.p = hoaTietNgoiSaoCau->mauNen.p;
   }
   
   return mau;
}

// Tìm R_c và θ_c
// R_0 = R_c/cos(θ_0 – θ_c)   R_1 = R_c/cos(θ_1 – θ_c)
void tinhBanKinhVaGocRanh( float *banKinhRanh, float *gocRanh , float banKinhNoi, float banKinhNgoai, float nuaGoc ) {
   unsigned char soLapLai = 0;
   unsigned char xong = kSAI;
   float gocThap = 0.0f;
   float gocCao = 3.1415926f;
   
   // ---- coi thử được xong sớm
   float chenhLech = banKinhNgoai*cosf( gocThap ) - banKinhNoi*cosf( nuaGoc - gocThap );
   if( chenhLech < 0.0f )
      chenhLech = -chenhLech;
   if( chenhLech < 0.0001f ) {
      xong = kDUNG;
      *gocRanh = gocThap;
      return;
   }
   
   chenhLech = banKinhNgoai*cosf( gocCao ) - banKinhNoi*cosf( nuaGoc - gocCao );
   if( chenhLech < 0.0f )
      chenhLech = -chenhLech;
   if( chenhLech < 0.0001f ) {
      xong = kDUNG;
      *gocRanh = gocCao;
      return;
   }
   
   while( !xong ) {
      // ---- tính góc trung bình của gócCao và gócThấp
      float gocGiua = (gocThap + gocCao) * 0.5f;

      // ---- tính chênh lệch
      float giaTri = banKinhNgoai*cosf( gocGiua ) - banKinhNoi*cosf( nuaGoc - gocGiua );
      
      float chenhLech = giaTri;
      
      // ---- giá trị tuyệt đối
      if( chenhLech < 0.0f )
         chenhLech = -chenhLech;
      if( chenhLech < 0.0001f ) {
         xong = kDUNG;
         *gocRanh = gocGiua;
      }
      
      if( giaTri > 0.0f ) {
         gocThap = gocGiua;
      }
      else {
         gocCao = gocGiua;
      }
      soLapLai++;
      
      if( soLapLai > 20 )
         xong = kDUNG;
   }
   *banKinhRanh = cosf( *gocRanh );

}


#pragma mark ---- Họa Tiết Quăn Cầu
HoaTietQuan datHoaTietQuan( Mau *mauNen, Mau *mauThap, Mau *mauCao ) {
   HoaTietQuan hoaTietQuan;
   hoaTietQuan.mauNen.d = mauNen->d;
   hoaTietQuan.mauNen.l = mauNen->l;
   hoaTietQuan.mauNen.x = mauNen->x;
   hoaTietQuan.mauNen.dd = mauNen->dd;
   hoaTietQuan.mauNen.p = mauNen->p;
   
   Mau phamViMau;
   phamViMau.d = mauCao->d - mauThap->d;
   phamViMau.l = mauCao->l - mauThap->l;
   phamViMau.x = mauCao->x - mauThap->x;
   phamViMau.dd = mauCao->dd - mauThap->dd;
   phamViMau.p = mauCao->p - mauThap->p;
   
   hoaTietQuan.tanSo = (rand() & 0x07) + 2;
   hoaTietQuan.soLuongQuan = (rand() & 0x0f) + 1;
   hoaTietQuan.beRongQuan = rand()/kSO_NGUYEN_TOI_DA;

   if( hoaTietQuan.soLuongQuan < 5 )
      hoaTietQuan.soLuongQuan = 5;
   
   unsigned char soQuan = 0;
   while( soQuan < hoaTietQuan.soLuongQuan ) {
      hoaTietQuan.mangMau[soQuan].d = phamViMau.d*rand()/kSO_NGUYEN_TOI_DA + mauThap->d;
      hoaTietQuan.mangMau[soQuan].l = phamViMau.l*rand()/kSO_NGUYEN_TOI_DA + mauThap->l;
      hoaTietQuan.mangMau[soQuan].x = phamViMau.x*rand()/kSO_NGUYEN_TOI_DA + mauThap->x;
      hoaTietQuan.mangMau[soQuan].dd = phamViMau.dd*rand()/kSO_NGUYEN_TOI_DA + mauThap->dd;
      hoaTietQuan.mangMau[soQuan].p = phamViMau.p*rand()/kSO_NGUYEN_TOI_DA + mauThap->p;
      soQuan++;
   }

   return hoaTietQuan;
}

Mau hoaTietQuan( Vecto *viTri, HoaTietQuan *hoaTietQuan ) {

   // ---- tính bán kính
   float banKinh = sqrtf( viTri->x*viTri->x + viTri->z*viTri->z );
   float viDo = atanf( viTri->y/banKinh );
   
   // ---- tính kính độ
   float kinhDo = 0.0f;
   if( banKinh != 0.0f )
      kinhDo = atan( viTri->z/viTri->x );
   
   kinhDo += 0.1f*sinf( hoaTietQuan->tanSo*viDo );
   
   // ---- giữ 0 ≤ kinh độ ≤ 2π
   if( viTri->x < 0.0f )
      kinhDo += 3.141592f;
   else
      if( viTri->z < 0.0f )
         kinhDo += 6.283184f;

   // ---- tính số quăn thực
   float soQuanThuc = kinhDo*hoaTietQuan->soLuongQuan/6.283184f;

   unsigned char soQuan = floor(soQuanThuc);
   Mau mau;
   if( (soQuanThuc - soQuan) < hoaTietQuan->beRongQuan ) {
      mau.d = hoaTietQuan->mauNen.d;
      mau.l = hoaTietQuan->mauNen.l;
      mau.x = hoaTietQuan->mauNen.x;
      mau.dd = hoaTietQuan->mauNen.dd;
      mau.p = hoaTietQuan->mauNen.p;
   }
   else {
      mau.d = hoaTietQuan->mangMau[soQuan].d;
      mau.l = hoaTietQuan->mangMau[soQuan].l;
      mau.x = hoaTietQuan->mangMau[soQuan].x;
      mau.dd = hoaTietQuan->mangMau[soQuan].dd;
      mau.p = hoaTietQuan->mangMau[soQuan].p;
   }
   
   return mau;
}

#pragma mark ---- Họa Tiết Quần Xoay
HoaTietQuanXoay datHoaTietQuanXoay( Mau *mauNen, Mau *mauQuan0, Mau *mauQuan1, Mau *mauQuan2, float phanQuan0, float phanQuan1, float phanQuan2, float xoay, float dich, unsigned char soLapVong ) {
   HoaTietQuanXoay hoaTietQuanXoay;
   // ---- màu nền
   hoaTietQuanXoay.mauNen.d = mauNen->d;
   hoaTietQuanXoay.mauNen.l = mauNen->l;
   hoaTietQuanXoay.mauNen.x = mauNen->x;
   hoaTietQuanXoay.mauNen.dd = mauNen->dd;
   hoaTietQuanXoay.mauNen.p = mauNen->p;
   // ---- màu quằn 0
   hoaTietQuanXoay.mauQuan0.d = mauQuan0->d;
   hoaTietQuanXoay.mauQuan0.l = mauQuan0->l;
   hoaTietQuanXoay.mauQuan0.x = mauQuan0->x;
   hoaTietQuanXoay.mauQuan0.dd = mauQuan0->dd;
   hoaTietQuanXoay.mauQuan0.p = mauQuan0->p;
   // ---- màu quằn 1
   hoaTietQuanXoay.mauQuan1.d = mauQuan1->d;
   hoaTietQuanXoay.mauQuan1.l = mauQuan1->l;
   hoaTietQuanXoay.mauQuan1.x = mauQuan1->x;
   hoaTietQuanXoay.mauQuan1.dd = mauQuan1->dd;
   hoaTietQuanXoay.mauQuan1.p = mauQuan1->p;
   // ---- màu quằn 1
   hoaTietQuanXoay.mauQuan2.d = mauQuan2->d;
   hoaTietQuanXoay.mauQuan2.l = mauQuan2->l;
   hoaTietQuanXoay.mauQuan2.x = mauQuan2->x;
   hoaTietQuanXoay.mauQuan2.dd = mauQuan2->dd;
   hoaTietQuanXoay.mauQuan2.p = mauQuan2->p;
   // ---- phân số cho quằn
   hoaTietQuanXoay.phanQuan0 = phanQuan0;
   hoaTietQuanXoay.phanQuan1 = phanQuan1;
   hoaTietQuanXoay.phanQuan2 = phanQuan2;
   // ---- xoay
   hoaTietQuanXoay.xoay = xoay;
   // ---- dịch xoay
   hoaTietQuanXoay.dich = dich;
   // ---- tính đoạn
   if( soLapVong < 2 )
      soLapVong = 2;
   hoaTietQuanXoay.soLapVong = soLapVong;
   hoaTietQuanXoay.doan = 6.283184f/(float)soLapVong;
   return hoaTietQuanXoay;
}

Mau hoaTietQuanXoay( Vecto *viTri, HoaTietQuanXoay *hoaTietQuanXoay ) {
   
   // ---- tính góc
   float banKinh = viTri->x*viTri->x + viTri->z*viTri->z;

   // ---- tính góc
   float goc = 0.0f;
   if( banKinh != 0.0f )
      goc = atan( viTri->z/viTri->x );
   
   // ---- giữ 0 ≤ góc ≤ 2π
   if( viTri->x < 0.0f )
      goc += 3.141592f;
   else
      if( viTri->z < 0.0f )
         goc += 6.283184f;
   
   goc += hoaTietQuanXoay->xoay*viTri->y + hoaTietQuanXoay->dich;
   
   // ---- 0,0 đến 1,0
   goc /= hoaTietQuanXoay->doan;
   // ---- cần phần số
   goc -= floor(goc);

   Mau mau;
   if( goc < 0.333333f ) {
      if( goc < 0.333333f - hoaTietQuanXoay->phanQuan0 ) {
         mau.d = hoaTietQuanXoay->mauNen.d;
         mau.l = hoaTietQuanXoay->mauNen.l;
         mau.x = hoaTietQuanXoay->mauNen.x;
         mau.dd = hoaTietQuanXoay->mauNen.dd;
         mau.p = hoaTietQuanXoay->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanXoay->mauQuan0.d;
         mau.l = hoaTietQuanXoay->mauQuan0.l;
         mau.x = hoaTietQuanXoay->mauQuan0.x;
         mau.dd = hoaTietQuanXoay->mauQuan0.dd;
         mau.p = hoaTietQuanXoay->mauQuan0.p;
      }
   }
   else if( goc < 0.666667f ) {
      if( goc < 0.666667f - hoaTietQuanXoay->phanQuan1 ) {
         mau.d = hoaTietQuanXoay->mauNen.d;
         mau.l = hoaTietQuanXoay->mauNen.l;
         mau.x = hoaTietQuanXoay->mauNen.x;
         mau.dd = hoaTietQuanXoay->mauNen.dd;
         mau.p = hoaTietQuanXoay->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanXoay->mauQuan1.d;
         mau.l = hoaTietQuanXoay->mauQuan1.l;
         mau.x = hoaTietQuanXoay->mauQuan1.x;
         mau.dd = hoaTietQuanXoay->mauQuan1.dd;
         mau.p = hoaTietQuanXoay->mauQuan1.p;
      }
   }
   else {
      if( goc < 1.0f - hoaTietQuanXoay->phanQuan2 ) {
         mau.d = hoaTietQuanXoay->mauNen.d;
         mau.l = hoaTietQuanXoay->mauNen.l;
         mau.x = hoaTietQuanXoay->mauNen.x;
         mau.dd = hoaTietQuanXoay->mauNen.dd;
         mau.p = hoaTietQuanXoay->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanXoay->mauQuan2.d;
         mau.l = hoaTietQuanXoay->mauQuan2.l;
         mau.x = hoaTietQuanXoay->mauQuan2.x;
         mau.dd = hoaTietQuanXoay->mauQuan2.dd;
         mau.p = hoaTietQuanXoay->mauQuan2.p;
      }
   }
   
   return mau;
}

#pragma mark ---- Họa Tiết Quần Sóng Theo Hướng (vuông góc với hướng)
HoaTietQuanSongTheoHuong datHoaTietQuanSongTheoHuong( Vecto *huongNgang, Vecto *huongDoc, Mau *mauNen, Mau *mauQuan0, Mau *mauQuan1, Mau *mauQuan2, float phanQuan0, float phanQuan1, float phanQuan2, float tanSoNgang, float tanSoToi, float bienDo,
                                                     float dichDoan, float doan ) {

   HoaTietQuanSongTheoHuong hoaTietQuanSongTheoHuong;
   // ---- hướng, hướng ngang và dọc phải vuông góc nhau
   hoaTietQuanSongTheoHuong.huongNgang.x = huongNgang->x;
   hoaTietQuanSongTheoHuong.huongNgang.y = huongNgang->y;
   hoaTietQuanSongTheoHuong.huongNgang.z = huongNgang->z;

   hoaTietQuanSongTheoHuong.huongDoc.x = huongDoc->x;
   hoaTietQuanSongTheoHuong.huongDoc.y = huongDoc->y;
   hoaTietQuanSongTheoHuong.huongDoc.z = huongDoc->z;

   hoaTietQuanSongTheoHuong.huongToi = tichCoHuong( huongNgang, huongDoc );
//   printf( "huongToi %5.3f %5.3f %5.3f\n", hoaTietQuanSongTheoHuong.huongToi.x, hoaTietQuanSongTheoHuong.huongToi.y, hoaTietQuanSongTheoHuong.huongToi.z );
   
   // ---- màu nền
   hoaTietQuanSongTheoHuong.mauNen.d = mauNen->d;
   hoaTietQuanSongTheoHuong.mauNen.l = mauNen->l;
   hoaTietQuanSongTheoHuong.mauNen.x = mauNen->x;
   hoaTietQuanSongTheoHuong.mauNen.dd = mauNen->dd;
   hoaTietQuanSongTheoHuong.mauNen.p = mauNen->p;
   // ---- màu quằn 0
   hoaTietQuanSongTheoHuong.mauQuan0.d = mauQuan0->d;
   hoaTietQuanSongTheoHuong.mauQuan0.l = mauQuan0->l;
   hoaTietQuanSongTheoHuong.mauQuan0.x = mauQuan0->x;
   hoaTietQuanSongTheoHuong.mauQuan0.dd = mauQuan0->dd;
   hoaTietQuanSongTheoHuong.mauQuan0.p = mauQuan0->p;
   // ---- màu quằn 1
   hoaTietQuanSongTheoHuong.mauQuan1.d = mauQuan1->d;
   hoaTietQuanSongTheoHuong.mauQuan1.l = mauQuan1->l;
   hoaTietQuanSongTheoHuong.mauQuan1.x = mauQuan1->x;
   hoaTietQuanSongTheoHuong.mauQuan1.dd = mauQuan1->dd;
   hoaTietQuanSongTheoHuong.mauQuan1.p = mauQuan1->p;
   // ---- màu quằn 1
   hoaTietQuanSongTheoHuong.mauQuan2.d = mauQuan2->d;
   hoaTietQuanSongTheoHuong.mauQuan2.l = mauQuan2->l;
   hoaTietQuanSongTheoHuong.mauQuan2.x = mauQuan2->x;
   hoaTietQuanSongTheoHuong.mauQuan2.dd = mauQuan2->dd;
   hoaTietQuanSongTheoHuong.mauQuan2.p = mauQuan2->p;
   // ---- phân số cho quằn
   hoaTietQuanSongTheoHuong.phanQuan0 = phanQuan0;
   hoaTietQuanSongTheoHuong.phanQuan1 = phanQuan1;
   hoaTietQuanSongTheoHuong.phanQuan2 = phanQuan2;
   // ---- tần số
   hoaTietQuanSongTheoHuong.tanSoNgang = tanSoNgang;
   hoaTietQuanSongTheoHuong.tanSoToi = tanSoToi;
   // ---- biên độ
   hoaTietQuanSongTheoHuong.bienDo = bienDo;
   // ---- dịch xoay
   hoaTietQuanSongTheoHuong.dich = dichDoan;
   // ---- không cho đoạn = 0
   if( doan == 0.0f )
      doan = 1.0f;
   hoaTietQuanSongTheoHuong.doan = doan;
   
   return hoaTietQuanSongTheoHuong;
}

Mau hoaTietQuanSongTheoHuong( Vecto *viTri, HoaTietQuanSongTheoHuong *hoaTietQuanSongTheoHuong ) {

   // ---- đoạn
   float doan = viTri->x*hoaTietQuanSongTheoHuong->huongDoc.x + viTri->y*hoaTietQuanSongTheoHuong->huongDoc.y +
                       viTri->z*hoaTietQuanSongTheoHuong->huongDoc.z;
   
   // ---- hai góc cho sinf()
   float gocNgang = viTri->x*hoaTietQuanSongTheoHuong->huongNgang.x + viTri->y*hoaTietQuanSongTheoHuong->huongNgang.y +
   viTri->z*hoaTietQuanSongTheoHuong->huongNgang.z;
   
   float gocToi = viTri->x*hoaTietQuanSongTheoHuong->huongToi.x + viTri->y*hoaTietQuanSongTheoHuong->huongToi.y +
   viTri->z*hoaTietQuanSongTheoHuong->huongToi.z;

   // ---- xem cần tính sin
   if( hoaTietQuanSongTheoHuong->bienDo == 0.0f )
      doan += hoaTietQuanSongTheoHuong->dich;
   else
      doan += hoaTietQuanSongTheoHuong->dich + hoaTietQuanSongTheoHuong->bienDo*cosf( gocNgang*hoaTietQuanSongTheoHuong->tanSoNgang )*cosf( gocToi*hoaTietQuanSongTheoHuong->tanSoToi );

   // ---- 0,0 đến 1,0
   float doanDonViHoa = doan / hoaTietQuanSongTheoHuong->doan;
   // ---- cần phần số
   doanDonViHoa -= floor(doanDonViHoa);
   if( doanDonViHoa > 1.0f ){
      printf( "hoaTietQuanSongTheoHuong: đoánĐơnVịHóa > 1.0f\n" );
      exit(0);
   }

   Mau mau;
   if( doanDonViHoa < 0.333333f ) {
      if( doanDonViHoa < 0.333333f - hoaTietQuanSongTheoHuong->phanQuan0 ) {
         mau.d = hoaTietQuanSongTheoHuong->mauNen.d;
         mau.l = hoaTietQuanSongTheoHuong->mauNen.l;
         mau.x = hoaTietQuanSongTheoHuong->mauNen.x;
         mau.dd = hoaTietQuanSongTheoHuong->mauNen.dd;
         mau.p = hoaTietQuanSongTheoHuong->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanSongTheoHuong->mauQuan0.d;
         mau.l = hoaTietQuanSongTheoHuong->mauQuan0.l;
         mau.x = hoaTietQuanSongTheoHuong->mauQuan0.x;
         mau.dd = hoaTietQuanSongTheoHuong->mauQuan0.dd;
         mau.p = hoaTietQuanSongTheoHuong->mauQuan0.p;
      }
   }
   else if( doanDonViHoa < 0.666667f ) {
      if( doanDonViHoa < 0.666667f - hoaTietQuanSongTheoHuong->phanQuan1 ) {
         mau.d = hoaTietQuanSongTheoHuong->mauNen.d;
         mau.l = hoaTietQuanSongTheoHuong->mauNen.l;
         mau.x = hoaTietQuanSongTheoHuong->mauNen.x;
         mau.dd = hoaTietQuanSongTheoHuong->mauNen.dd;
         mau.p = hoaTietQuanSongTheoHuong->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanSongTheoHuong->mauQuan1.d;
         mau.l = hoaTietQuanSongTheoHuong->mauQuan1.l;
         mau.x = hoaTietQuanSongTheoHuong->mauQuan1.x;
         mau.dd = hoaTietQuanSongTheoHuong->mauQuan1.dd;
         mau.p = hoaTietQuanSongTheoHuong->mauQuan1.p;
      }
   }
   else {
      if( doanDonViHoa < 1.0f - hoaTietQuanSongTheoHuong->phanQuan2 ) {
         mau.d = hoaTietQuanSongTheoHuong->mauNen.d;
         mau.l = hoaTietQuanSongTheoHuong->mauNen.l;
         mau.x = hoaTietQuanSongTheoHuong->mauNen.x;
         mau.dd = hoaTietQuanSongTheoHuong->mauNen.dd;
         mau.p = hoaTietQuanSongTheoHuong->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanSongTheoHuong->mauQuan2.d;
         mau.l = hoaTietQuanSongTheoHuong->mauQuan2.l;
         mau.x = hoaTietQuanSongTheoHuong->mauQuan2.x;
         mau.dd = hoaTietQuanSongTheoHuong->mauQuan2.dd;
         mau.p = hoaTietQuanSongTheoHuong->mauQuan2.p;
      }
   }
   
   return mau;
}

#pragma mark ---- Họa Tiết Quần Sóng Trục Z
HoaTietQuanSongTrucZ datHoaTietQuanSongTrucZ( Mau *mauNen, Mau *mauQuan0, Mau *mauQuan1, Mau *mauQuan2, float phanQuan0, float phanQuan1, float phanQuan2, float tanSo, float bienDo, float dich, unsigned char soLapVong ) {

   HoaTietQuanSongTrucZ hoaTietQuanSongTrucZ;
   // ---- màu nền
   hoaTietQuanSongTrucZ.mauNen.d = mauNen->d;
   hoaTietQuanSongTrucZ.mauNen.l = mauNen->l;
   hoaTietQuanSongTrucZ.mauNen.x = mauNen->x;
   hoaTietQuanSongTrucZ.mauNen.dd = mauNen->dd;
   hoaTietQuanSongTrucZ.mauNen.p = mauNen->p;
   // ---- màu quằn 0
   hoaTietQuanSongTrucZ.mauQuan0.d = mauQuan0->d;
   hoaTietQuanSongTrucZ.mauQuan0.l = mauQuan0->l;
   hoaTietQuanSongTrucZ.mauQuan0.x = mauQuan0->x;
   hoaTietQuanSongTrucZ.mauQuan0.dd = mauQuan0->dd;
   hoaTietQuanSongTrucZ.mauQuan0.p = mauQuan0->p;
   // ---- màu quằn 1
   hoaTietQuanSongTrucZ.mauQuan1.d = mauQuan1->d;
   hoaTietQuanSongTrucZ.mauQuan1.l = mauQuan1->l;
   hoaTietQuanSongTrucZ.mauQuan1.x = mauQuan1->x;
   hoaTietQuanSongTrucZ.mauQuan1.dd = mauQuan1->dd;
   hoaTietQuanSongTrucZ.mauQuan1.p = mauQuan1->p;
   // ---- màu quằn 1
   hoaTietQuanSongTrucZ.mauQuan2.d = mauQuan2->d;
   hoaTietQuanSongTrucZ.mauQuan2.l = mauQuan2->l;
   hoaTietQuanSongTrucZ.mauQuan2.x = mauQuan2->x;
   hoaTietQuanSongTrucZ.mauQuan2.dd = mauQuan2->dd;
   hoaTietQuanSongTrucZ.mauQuan2.p = mauQuan2->p;
   // ---- phân số cho quằn
   hoaTietQuanSongTrucZ.phanQuan0 = phanQuan0;
   hoaTietQuanSongTrucZ.phanQuan1 = phanQuan1;
   hoaTietQuanSongTrucZ.phanQuan2 = phanQuan2;
   // ---- tần số
   hoaTietQuanSongTrucZ.tanSo = tanSo;
   // ---- biên độ
   hoaTietQuanSongTrucZ.bienDo = bienDo;
   // ---- dịch xoay
   hoaTietQuanSongTrucZ.dich = dich;
   // ---- tính đoạn
   if( soLapVong < 2 )
      soLapVong = 2;
   hoaTietQuanSongTrucZ.soLapVong = soLapVong;
   hoaTietQuanSongTrucZ.doan = 6.283184f/(float)soLapVong;
   return hoaTietQuanSongTrucZ;
}

Mau hoaTietQuanSongTrucZ( Vecto *viTri, HoaTietQuanSongTrucZ *hoaTietQuanSongTrucZ ) {
   
   // ---- tính góc
   float banKinh = viTri->x*viTri->x + viTri->z*viTri->z;
   
   // ---- tính góc
   float goc = 0.0f;
   if( banKinh != 0.0f )
      goc = atan( viTri->z/viTri->x );
   
   // ---- giữ 0 ≤ góc ≤ 2π
   if( viTri->x < 0.0f )
      goc += 3.141592f;
   else
      if( viTri->z < 0.0f )
         goc += 6.283184f;
   
   goc += hoaTietQuanSongTrucZ->dich + hoaTietQuanSongTrucZ->bienDo*sinf( viTri->y*hoaTietQuanSongTrucZ->tanSo );
   if( goc < 0.0f )
      goc += 6.283184f;
   else if( goc > 6.283184f )
      goc -= 6.283184f;
   
   // ---- 0,0 đến 1,0
   goc /= hoaTietQuanSongTrucZ->doan;
   // ---- cần phần số
   goc -= floor(goc);
   if( goc > 1.0f )
      exit(0);
   
   Mau mau;
   if( goc < 0.333333f ) {
      if( goc < 0.333333f - hoaTietQuanSongTrucZ->phanQuan0 ) {
         mau.d = hoaTietQuanSongTrucZ->mauNen.d;
         mau.l = hoaTietQuanSongTrucZ->mauNen.l;
         mau.x = hoaTietQuanSongTrucZ->mauNen.x;
         mau.dd = hoaTietQuanSongTrucZ->mauNen.dd;
         mau.p = hoaTietQuanSongTrucZ->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanSongTrucZ->mauQuan0.d;
         mau.l = hoaTietQuanSongTrucZ->mauQuan0.l;
         mau.x = hoaTietQuanSongTrucZ->mauQuan0.x;
         mau.dd = hoaTietQuanSongTrucZ->mauQuan0.dd;
         mau.p = hoaTietQuanSongTrucZ->mauQuan0.p;
      }
   }
   else if( goc < 0.666667f ) {
      if( goc < 0.666667f - hoaTietQuanSongTrucZ->phanQuan1 ) {
         mau.d = hoaTietQuanSongTrucZ->mauNen.d;
         mau.l = hoaTietQuanSongTrucZ->mauNen.l;
         mau.x = hoaTietQuanSongTrucZ->mauNen.x;
         mau.dd = hoaTietQuanSongTrucZ->mauNen.dd;
         mau.p = hoaTietQuanSongTrucZ->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanSongTrucZ->mauQuan1.d;
         mau.l = hoaTietQuanSongTrucZ->mauQuan1.l;
         mau.x = hoaTietQuanSongTrucZ->mauQuan1.x;
         mau.dd = hoaTietQuanSongTrucZ->mauQuan1.dd;
         mau.p = hoaTietQuanSongTrucZ->mauQuan1.p;
      }
   }
   else {
      if( goc < 1.0f - hoaTietQuanSongTrucZ->phanQuan2 ) {
         mau.d = hoaTietQuanSongTrucZ->mauNen.d;
         mau.l = hoaTietQuanSongTrucZ->mauNen.l;
         mau.x = hoaTietQuanSongTrucZ->mauNen.x;
         mau.dd = hoaTietQuanSongTrucZ->mauNen.dd;
         mau.p = hoaTietQuanSongTrucZ->mauNen.p;
      }
      else {
         mau.d = hoaTietQuanSongTrucZ->mauQuan2.d;
         mau.l = hoaTietQuanSongTrucZ->mauQuan2.l;
         mau.x = hoaTietQuanSongTrucZ->mauQuan2.x;
         mau.dd = hoaTietQuanSongTrucZ->mauQuan2.dd;
         mau.p = hoaTietQuanSongTrucZ->mauQuan2.p;
      }
   }
   
   return mau;
}

#pragma mark ---- Họa Tiết Quằn Sóng Tia Phai
HoaTietQuanSongTiaPhai datHoaTietQuanSongTiaPhai( Mau *mauNen, Mau *mauQuan0, Mau *mauQuan1, Mau *mauQuan2, float phanQuan0, float phanQuan1, float phanQuan2, float tanSo, float bienDo, float dich, unsigned char soLapVong, float banKinhBatDauPhai, float banKinhKetThucPhai ) {

   HoaTietQuanSongTiaPhai hoaTietQuanSongTiaPhai;
   // ---- màu nền
   hoaTietQuanSongTiaPhai.mauNen.d = mauNen->d;
   hoaTietQuanSongTiaPhai.mauNen.l = mauNen->l;
   hoaTietQuanSongTiaPhai.mauNen.x = mauNen->x;
   hoaTietQuanSongTiaPhai.mauNen.dd = mauNen->dd;
   hoaTietQuanSongTiaPhai.mauNen.p = mauNen->p;
   // ---- màu quằn 0
   hoaTietQuanSongTiaPhai.mauQuan0.d = mauQuan0->d;
   hoaTietQuanSongTiaPhai.mauQuan0.l = mauQuan0->l;
   hoaTietQuanSongTiaPhai.mauQuan0.x = mauQuan0->x;
   hoaTietQuanSongTiaPhai.mauQuan0.dd = mauQuan0->dd;
   hoaTietQuanSongTiaPhai.mauQuan0.p = mauQuan0->p;
   // ---- màu quằn 1
   hoaTietQuanSongTiaPhai.mauQuan1.d = mauQuan1->d;
   hoaTietQuanSongTiaPhai.mauQuan1.l = mauQuan1->l;
   hoaTietQuanSongTiaPhai.mauQuan1.x = mauQuan1->x;
   hoaTietQuanSongTiaPhai.mauQuan1.dd = mauQuan1->dd;
   hoaTietQuanSongTiaPhai.mauQuan1.p = mauQuan1->p;
   // ---- màu quằn 1
   hoaTietQuanSongTiaPhai.mauQuan2.d = mauQuan2->d;
   hoaTietQuanSongTiaPhai.mauQuan2.l = mauQuan2->l;
   hoaTietQuanSongTiaPhai.mauQuan2.x = mauQuan2->x;
   hoaTietQuanSongTiaPhai.mauQuan2.dd = mauQuan2->dd;
   hoaTietQuanSongTiaPhai.mauQuan2.p = mauQuan2->p;
   // ---- phân số cho quằn
   hoaTietQuanSongTiaPhai.phanQuan0 = phanQuan0;
   hoaTietQuanSongTiaPhai.phanQuan1 = phanQuan1;
   hoaTietQuanSongTiaPhai.phanQuan2 = phanQuan2;
   // ---- tần số
   hoaTietQuanSongTiaPhai.tanSo = tanSo;
   // ---- biên độ
   hoaTietQuanSongTiaPhai.bienDo = bienDo;
   // ---- dịch xoay
   hoaTietQuanSongTiaPhai.dich = dich;
   // ---- tính đoạn
   if( soLapVong < 2 )
      soLapVong = 2;
   hoaTietQuanSongTiaPhai.soLapVong = soLapVong;
   hoaTietQuanSongTiaPhai.doanVong = 6.283184f/(float)soLapVong;
   
   // ---- bán kính bắt đầu và kết thúc phai màu
   hoaTietQuanSongTiaPhai.banKinhBatDauPhai = banKinhBatDauPhai*banKinhBatDauPhai;
   hoaTietQuanSongTiaPhai.banKinhKetThucPhai = banKinhKetThucPhai*banKinhKetThucPhai;
   hoaTietQuanSongTiaPhai.doanPhai = hoaTietQuanSongTiaPhai.banKinhKetThucPhai - hoaTietQuanSongTiaPhai.banKinhBatDauPhai;

   return hoaTietQuanSongTiaPhai;
}

Mau hoaTietQuanSongTiaPhai( Vecto *viTri, HoaTietQuanSongTiaPhai *hoaTietQuanSongTiaPhai ) {
   
   // ---- tính góc
   float banKinh = viTri->x*viTri->x + viTri->z*viTri->z;
   
   // ---- tính góc
   float goc = 0.0f;
   if( banKinh != 0.0f )
      goc = atan( viTri->z/viTri->x );
   
   // ---- giữ 0 ≤ góc ≤ 2π
   if( viTri->x < 0.0f )
      goc += 3.141592f;
   else
      if( viTri->z < 0.0f )
         goc += 6.283184f;
   
   goc += hoaTietQuanSongTiaPhai->dich + hoaTietQuanSongTiaPhai->bienDo*sinf( banKinh*hoaTietQuanSongTiaPhai->tanSo );
   if( goc < 0.0f )
      goc += 6.283184f;
   else if( goc > 6.283184f )
      goc -= 6.283184f;
   
   // ---- 0,0 đến 1,0
   goc /= hoaTietQuanSongTiaPhai->doanVong;
   // ---- cần phần số
   goc -= floor(goc);
   if( goc > 1.0f ){
      printf( "hoaTietQuanSongTiaPhai: góc > 1\n" );
      exit(0);
   }
   // ---- tìm màu
   Mau mau;
   if( goc < 0.333333f ) {

      if( (goc < 0.333333f - hoaTietQuanSongTiaPhai->phanQuan0) || (banKinh > hoaTietQuanSongTiaPhai->banKinhKetThucPhai) ) {
         mau.d = hoaTietQuanSongTiaPhai->mauNen.d;
         mau.l = hoaTietQuanSongTiaPhai->mauNen.l;
         mau.x = hoaTietQuanSongTiaPhai->mauNen.x;
         mau.dd = hoaTietQuanSongTiaPhai->mauNen.dd;
         mau.p = hoaTietQuanSongTiaPhai->mauNen.p;
      }
      else if( banKinh < hoaTietQuanSongTiaPhai->banKinhBatDauPhai ) {
         mau.d = hoaTietQuanSongTiaPhai->mauQuan0.d;
         mau.l = hoaTietQuanSongTiaPhai->mauQuan0.l;
         mau.x = hoaTietQuanSongTiaPhai->mauQuan0.x;
         mau.dd = hoaTietQuanSongTiaPhai->mauQuan0.dd;
         mau.p = hoaTietQuanSongTiaPhai->mauQuan0.p;
      }
      else {
         float tiSoMauSong = (banKinh - hoaTietQuanSongTiaPhai->banKinhBatDauPhai)/hoaTietQuanSongTiaPhai->doanPhai;
         float nghichTiSoMauSong = 1.0f - tiSoMauSong;
         mau.d = hoaTietQuanSongTiaPhai->mauNen.d*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan0.d*nghichTiSoMauSong;
         mau.l = hoaTietQuanSongTiaPhai->mauNen.l*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan0.l*nghichTiSoMauSong;
         mau.x = hoaTietQuanSongTiaPhai->mauNen.x*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan0.x*nghichTiSoMauSong;
         mau.dd = hoaTietQuanSongTiaPhai->mauNen.dd*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan0.dd*nghichTiSoMauSong;
         mau.p = hoaTietQuanSongTiaPhai->mauNen.p*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan0.p*nghichTiSoMauSong;
      }
   }
   else if( goc < 0.666667f ) {
      if( (goc < 0.666667f - hoaTietQuanSongTiaPhai->phanQuan1) || (banKinh > hoaTietQuanSongTiaPhai->banKinhKetThucPhai) ) {
         mau.d = hoaTietQuanSongTiaPhai->mauNen.d;
         mau.l = hoaTietQuanSongTiaPhai->mauNen.l;
         mau.x = hoaTietQuanSongTiaPhai->mauNen.x;
         mau.dd = hoaTietQuanSongTiaPhai->mauNen.dd;
         mau.p = hoaTietQuanSongTiaPhai->mauNen.p;
      }
      else if( banKinh < hoaTietQuanSongTiaPhai->banKinhBatDauPhai ) {
         mau.d = hoaTietQuanSongTiaPhai->mauQuan1.d;
         mau.l = hoaTietQuanSongTiaPhai->mauQuan1.l;
         mau.x = hoaTietQuanSongTiaPhai->mauQuan1.x;
         mau.dd = hoaTietQuanSongTiaPhai->mauQuan1.dd;
         mau.p = hoaTietQuanSongTiaPhai->mauQuan1.p;
      }
      else {
         float tiSoMauSong = (banKinh - hoaTietQuanSongTiaPhai->banKinhBatDauPhai)/hoaTietQuanSongTiaPhai->doanPhai;
         float nghichTiSoMauSong = 1.0f - tiSoMauSong;
         mau.d = hoaTietQuanSongTiaPhai->mauNen.d*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan1.d*nghichTiSoMauSong;
         mau.l = hoaTietQuanSongTiaPhai->mauNen.l*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan1.l*nghichTiSoMauSong;
         mau.x = hoaTietQuanSongTiaPhai->mauNen.x*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan1.x*nghichTiSoMauSong;
         mau.dd = hoaTietQuanSongTiaPhai->mauNen.dd*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan1.dd*nghichTiSoMauSong;
         mau.p = hoaTietQuanSongTiaPhai->mauNen.p*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan1.p*nghichTiSoMauSong;
      }
   }
   else {
      if( (goc < 1.0f - hoaTietQuanSongTiaPhai->phanQuan2) || (banKinh > hoaTietQuanSongTiaPhai->banKinhKetThucPhai) ) {
         mau.d = hoaTietQuanSongTiaPhai->mauNen.d;
         mau.l = hoaTietQuanSongTiaPhai->mauNen.l;
         mau.x = hoaTietQuanSongTiaPhai->mauNen.x;
         mau.dd = hoaTietQuanSongTiaPhai->mauNen.dd;
         mau.p = hoaTietQuanSongTiaPhai->mauNen.p;
      }
      else if( banKinh < hoaTietQuanSongTiaPhai->banKinhBatDauPhai ) {
         mau.d = hoaTietQuanSongTiaPhai->mauQuan2.d;
         mau.l = hoaTietQuanSongTiaPhai->mauQuan2.l;
         mau.x = hoaTietQuanSongTiaPhai->mauQuan2.x;
         mau.dd = hoaTietQuanSongTiaPhai->mauQuan2.dd;
         mau.p = hoaTietQuanSongTiaPhai->mauQuan2.p;
      }
      else {
         float tiSoMauSong = (banKinh - hoaTietQuanSongTiaPhai->banKinhBatDauPhai)/hoaTietQuanSongTiaPhai->doanPhai;
         float nghichTiSoMauSong = 1.0f - tiSoMauSong;
         mau.d = hoaTietQuanSongTiaPhai->mauNen.d*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan2.d*nghichTiSoMauSong;
         mau.l = hoaTietQuanSongTiaPhai->mauNen.l*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan2.l*nghichTiSoMauSong;
         mau.x = hoaTietQuanSongTiaPhai->mauNen.x*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan2.x*nghichTiSoMauSong;
         mau.dd = hoaTietQuanSongTiaPhai->mauNen.dd*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan2.dd*nghichTiSoMauSong;
         mau.p = hoaTietQuanSongTiaPhai->mauNen.p*tiSoMauSong + hoaTietQuanSongTiaPhai->mauQuan2.p*nghichTiSoMauSong;
      }
   }
   
   return mau;
}

#pragma mark ---- Họa Tiết Sọc
HoaTietSoc datHoaTietSoc( Mau *mauNen, Mau *mauSoc, float quangSoc, float phanSoSoc, unsigned char trucSoc ) {
   
   HoaTietSoc hoaTietSoc;
   hoaTietSoc.mauNen.d = mauNen->d;
   hoaTietSoc.mauNen.l = mauNen->l;
   hoaTietSoc.mauNen.x = mauNen->x;
   hoaTietSoc.mauNen.dd = mauNen->dd;
   hoaTietSoc.mauNen.p = mauNen->p;
   
   hoaTietSoc.mauSoc.d = mauSoc->d;
   hoaTietSoc.mauSoc.l = mauSoc->l;
   hoaTietSoc.mauSoc.x = mauSoc->x;
   hoaTietSoc.mauSoc.dd = mauSoc->dd;
   hoaTietSoc.mauSoc.p = mauSoc->p;
   
   hoaTietSoc.quangSoc = quangSoc;
   hoaTietSoc.phanSoSoc = phanSoSoc;
   hoaTietSoc.trucSoc = trucSoc;
   return hoaTietSoc;
}

Mau hoaTietSoc( Vecto *viTri, HoaTietSoc *hoaTietSoc ) {

   float phanSo;

   if( hoaTietSoc->trucSoc == kTRUC_X ) {
      float soSoc = viTri->x/hoaTietSoc->quangSoc;
      phanSo = soSoc - floor( soSoc );
   }
   else if( hoaTietSoc->trucSoc == kTRUC_Y ) {
      float soSoc = viTri->y/hoaTietSoc->quangSoc;
      phanSo = soSoc - floor( soSoc );
   }
   else { //  if( hoaTietSoc->trucSoc == kTRUC_Z )
      float soSoc = viTri->z/hoaTietSoc->quangSoc;
      phanSo = soSoc - floor( soSoc );
   }
   // ---- chọn màu
   Mau mau;
   if( phanSo < hoaTietSoc->phanSoSoc )
      mau = hoaTietSoc->mauSoc;
   else
      mau = hoaTietSoc->mauNen;

//   printf( "phanSo %5.3f  hoaTietSoc->phanSoSoc %5.3f  mau %5.3f %5.3f %5.3f\n", phanSo, hoaTietSoc->phanSoSoc, mau.d, mau.l, mau.x );

   return mau;
}

#pragma mark ---- Họa Tiết Ca Rô Mịn
HoaTietCaRoMin datHoaTietCaRoMin( Mau *mau0, Mau *mau1, float beRong, float beCao, float beDai ) {
   HoaTietCaRoMin hoaTietCaRoMin;
   hoaTietCaRoMin.mau0.d = mau0->d;
   hoaTietCaRoMin.mau0.l = mau0->l;
   hoaTietCaRoMin.mau0.x = mau0->x;
   hoaTietCaRoMin.mau0.dd = mau0->dd;
   hoaTietCaRoMin.mau0.p = mau0->p;
   
   hoaTietCaRoMin.mau1.d = mau1->d;
   hoaTietCaRoMin.mau1.l = mau1->l;
   hoaTietCaRoMin.mau1.x = mau1->x;
   hoaTietCaRoMin.mau1.dd = mau1->dd;
   hoaTietCaRoMin.mau1.p = mau1->p;
   
   hoaTietCaRoMin.beRong = beRong;
   hoaTietCaRoMin.beCao = beCao;
   hoaTietCaRoMin.beDai = beDai;
   
   return hoaTietCaRoMin;
}

Mau hoaTietCaRoMin( Vecto *viTri, HoaTietCaRoMin *hoaTietCaRoMin ) {
   
   float x = (viTri->x)/hoaTietCaRoMin->beRong;
   float y = (viTri->y)/hoaTietCaRoMin->beCao;
   float z = (viTri->z)/hoaTietCaRoMin->beDai;
   // ---- lấy phần số
   float phanSoX = 6.283184f*(x - ceil(x));
   float phanSoY = 6.283184f*(y - ceil(y));  // int
   float phanSoZ = 6.283184f*(z - ceil(z));  // int
   
   float ketQua = 0.5f*(sinf( phanSoX ) * sinf( phanSoY ) * sinf( phanSoZ ) + 1.0f);
   float ketQuaNghich = 1.0f - ketQua;

   Mau mauCaRo;
   mauCaRo.d = ketQuaNghich*hoaTietCaRoMin->mau0.d + ketQua*hoaTietCaRoMin->mau1.d;
   mauCaRo.l = ketQuaNghich*hoaTietCaRoMin->mau0.l + ketQua*hoaTietCaRoMin->mau1.l;
   mauCaRo.x = ketQuaNghich*hoaTietCaRoMin->mau0.x + ketQua*hoaTietCaRoMin->mau1.x;
   mauCaRo.dd = ketQuaNghich*hoaTietCaRoMin->mau0.dd + ketQua*hoaTietCaRoMin->mau1.dd;
   mauCaRo.p = ketQuaNghich*hoaTietCaRoMin->mau0.p + ketQua*hoaTietCaRoMin->mau1.p;

   return mauCaRo;
}

#pragma mark ---- Họa Tiết Hai Chấm Bi
// ---- Hai chấm bi trong hình khối
// ---- hai vị trí cho chấm bi là tương đối tử góc 0; 0; 0 của hình khối
//          ^
//          |
// +--------+-------+
// |        |       |
// |        |góc    |
// |        +-------+---->
// |                |
// |                |
// +----------------+

HoaTietHaiChamBi datHoaTietHaiChamBi( Mau *mauNen, Mau *mau0, Mau *mau1, float banKinh0, float banKinh1, Vecto *viTri0, Vecto *viTri1, float beRong, float beCao, float beDai ) {

   HoaTietHaiChamBi hoaTietHaiChamBi;
   
   hoaTietHaiChamBi.viTri0.x = viTri0->x;
   hoaTietHaiChamBi.viTri0.y = viTri0->y;
   hoaTietHaiChamBi.viTri0.z = viTri0->z;

   hoaTietHaiChamBi.viTri1.x = viTri1->x;
   hoaTietHaiChamBi.viTri1.y = viTri1->y;
   hoaTietHaiChamBi.viTri1.z = viTri1->z;
   
   // ---- tính bán kính bình phương
   hoaTietHaiChamBi.banKinhBinh0 = banKinh0*banKinh0;
   hoaTietHaiChamBi.banKinhBinh1 = banKinh1*banKinh1;

   hoaTietHaiChamBi.mauNen.d = mauNen->d;
   hoaTietHaiChamBi.mauNen.l = mauNen->l;
   hoaTietHaiChamBi.mauNen.x = mauNen->x;
   hoaTietHaiChamBi.mauNen.dd = mauNen->dd;
   hoaTietHaiChamBi.mauNen.p = mauNen->p;
   
   hoaTietHaiChamBi.mau0.d = mau0->d;
   hoaTietHaiChamBi.mau0.l = mau0->l;
   hoaTietHaiChamBi.mau0.x = mau0->x;
   hoaTietHaiChamBi.mau0.dd = mau0->dd;
   hoaTietHaiChamBi.mau0.p = mau0->p;
   
   hoaTietHaiChamBi.mau1.d = mau1->d;
   hoaTietHaiChamBi.mau1.l = mau1->l;
   hoaTietHaiChamBi.mau1.x = mau1->x;
   hoaTietHaiChamBi.mau1.dd = mau1->dd;
   hoaTietHaiChamBi.mau1.p = mau1->p;
   
   hoaTietHaiChamBi.beRong = beRong;
   hoaTietHaiChamBi.beCao = beCao;
   hoaTietHaiChamBi.beDai = beDai;
   
   return hoaTietHaiChamBi;
}

Mau hoaTietHaiChamBi( Vecto *viTri, HoaTietHaiChamBi *hoaTietHaiChamBi ) {
   
   // ---- vị trí tương đối hình khối
   // ---- tính tương đối vơi tâm khối, vị trí củng tươg đối với tâm vật thể rồi
   // ---- tìm bao nhiêu khối lặp dòng từ floorf((viTri->x + hoaTietHaiChamBi->beRong*0.5f)/hoaTietHaiChamBi->beRong
   Vecto viTriTuongDoiKhoi;
   viTriTuongDoiKhoi.x = viTri->x - hoaTietHaiChamBi->beRong*floorf((viTri->x + hoaTietHaiChamBi->beRong*0.5f)/hoaTietHaiChamBi->beRong);
   viTriTuongDoiKhoi.y = viTri->y - hoaTietHaiChamBi->beCao*floorf((viTri->y + hoaTietHaiChamBi->beCao*0.5f)/hoaTietHaiChamBi->beCao);
   viTriTuongDoiKhoi.z = viTri->z - hoaTietHaiChamBi->beDai*floorf((viTri->z + hoaTietHaiChamBi->beDai*0.5f)/hoaTietHaiChamBi->beDai);

   // ---- xem trong hình cầu 0 hay không
   Vecto viTriTuongDoi0;
   viTriTuongDoi0.x = hoaTietHaiChamBi->viTri0.x - viTriTuongDoiKhoi.x;
   viTriTuongDoi0.y = hoaTietHaiChamBi->viTri0.y - viTriTuongDoiKhoi.y;
   viTriTuongDoi0.z = hoaTietHaiChamBi->viTri0.z - viTriTuongDoiKhoi.z;

   Mau mau;
   float banKinhBinh0 = viTriTuongDoi0.x*viTriTuongDoi0.x + viTriTuongDoi0.y*viTriTuongDoi0.y + viTriTuongDoi0.z*viTriTuongDoi0.z;

   if( banKinhBinh0 < hoaTietHaiChamBi->banKinhBinh0 ) {
      mau.d = hoaTietHaiChamBi->mau0.d;
      mau.l = hoaTietHaiChamBi->mau0.l;
      mau.x = hoaTietHaiChamBi->mau0.x;
      mau.dd = hoaTietHaiChamBi->mau0.dd;
      mau.p = hoaTietHaiChamBi->mau0.p;
   }
   else {
      // ---- xem trong hình cầu 1 hay không
      Vecto viTriTuongDoi1;
      viTriTuongDoi1.x = hoaTietHaiChamBi->viTri1.x - viTriTuongDoiKhoi.x;
      viTriTuongDoi1.y = hoaTietHaiChamBi->viTri1.y - viTriTuongDoiKhoi.y;
      viTriTuongDoi1.z = hoaTietHaiChamBi->viTri1.z - viTriTuongDoiKhoi.z;
      
      float banKinhBinh1 = viTriTuongDoi1.x*viTriTuongDoi1.x + viTriTuongDoi1.y*viTriTuongDoi1.y + viTriTuongDoi1.z*viTriTuongDoi1.z;
      if( banKinhBinh1 < hoaTietHaiChamBi->banKinhBinh1 ) {
         mau.d = hoaTietHaiChamBi->mau1.d;
         mau.l = hoaTietHaiChamBi->mau1.l;
         mau.x = hoaTietHaiChamBi->mau1.x;
         mau.dd = hoaTietHaiChamBi->mau1.dd;
         mau.p = hoaTietHaiChamBi->mau1.p;
      }
      // ---- †ô màu nền
      else {
         mau.d = hoaTietHaiChamBi->mauNen.d;
         mau.l = hoaTietHaiChamBi->mauNen.l;
         mau.x = hoaTietHaiChamBi->mauNen.x;
         mau.dd = hoaTietHaiChamBi->mauNen.dd;
         mau.p = hoaTietHaiChamBi->mauNen.p;
      }
   }

   return mau;
}

#pragma mark ---- Họa Tiết Bông Vòng
//    b.k. nội
// +-------------->| b.k. ngoại
// +--------------------->| b.k. vành
// +---------------------------->|
//                          vành   bông vòng
// +----------------------+------+--------+
HoaTietBongVong datHoaTietBongVong( Mau *mauNen, Mau *mauNoi, Mau *mauNgoai, Mau *mauVanh, float banKinhNoi, float banKinhNgoai, float banKinhVanh, Mau *mauBongVongNoi, Mau *mauBongVongNgoai, float tiSoToBong, unsigned short soLuongBong ) {
   
   HoaTietBongVong hoaTietBongVong;

   // ---- tính bán kính bình phương
   hoaTietBongVong.banKinhBinhNoi = banKinhNoi*banKinhNoi;
   hoaTietBongVong.banKinhBinhNgoai = banKinhNgoai*banKinhNgoai;
   hoaTietBongVong.banKinhBinhVanh = banKinhVanh*banKinhVanh;
   hoaTietBongVong.banKinhVanh = banKinhVanh;  // cần cho vẽ bông vòng
   
   hoaTietBongVong.mauNen.d = mauNen->d;
   hoaTietBongVong.mauNen.l = mauNen->l;
   hoaTietBongVong.mauNen.x = mauNen->x;
   hoaTietBongVong.mauNen.dd = mauNen->dd;
   hoaTietBongVong.mauNen.p = mauNen->p;
   
   hoaTietBongVong.mauNoi.d = mauNoi->d;
   hoaTietBongVong.mauNoi.l = mauNoi->l;
   hoaTietBongVong.mauNoi.x = mauNoi->x;
   hoaTietBongVong.mauNoi.dd = mauNoi->dd;
   hoaTietBongVong.mauNoi.p = mauNoi->p;
   
   hoaTietBongVong.mauNgoai.d = mauNgoai->d;
   hoaTietBongVong.mauNgoai.l = mauNgoai->l;
   hoaTietBongVong.mauNgoai.x = mauNgoai->x;
   hoaTietBongVong.mauNgoai.dd = mauNgoai->dd;
   hoaTietBongVong.mauNgoai.p = mauNgoai->p;
   
   hoaTietBongVong.mauVanh.d = mauVanh->d;
   hoaTietBongVong.mauVanh.l = mauVanh->l;
   hoaTietBongVong.mauVanh.x = mauVanh->x;
   hoaTietBongVong.mauVanh.dd = mauVanh->dd;
   hoaTietBongVong.mauVanh.p = mauVanh->p;
   
   if( soLuongBong < 4 )
      soLuongBong = 4;
   
   hoaTietBongVong.gocBongVong = 6.2831852f/(float)soLuongBong;
 
   // ---- tính bán kính bình cho các bông vòng
   if( tiSoToBong > 1.0f )
      tiSoToBong = 1.0f;

   float banKinhBongNgoai = 0.5f*banKinhVanh*6.2831852f/(float)soLuongBong;
   float banKinhBongNoi = banKinhBongNgoai*(1.0f - tiSoToBong );
   hoaTietBongVong.banKinhBinhBongVongNoi = banKinhBongNoi*banKinhBongNoi;
   hoaTietBongVong.banKinhBinhBongVongNgoai = banKinhBongNgoai*banKinhBongNgoai;

   hoaTietBongVong.mauBongVongNoi.d = mauBongVongNoi->d;
   hoaTietBongVong.mauBongVongNoi.l = mauBongVongNoi->l;
   hoaTietBongVong.mauBongVongNoi.x = mauBongVongNoi->x;
   hoaTietBongVong.mauBongVongNoi.dd = mauBongVongNoi->dd;
   hoaTietBongVong.mauBongVongNoi.p = mauBongVongNoi->p;
   
   hoaTietBongVong.mauBongVongNgoai.d = mauBongVongNgoai->d;
   hoaTietBongVong.mauBongVongNgoai.l = mauBongVongNgoai->l;
   hoaTietBongVong.mauBongVongNgoai.x = mauBongVongNgoai->x;
   hoaTietBongVong.mauBongVongNgoai.dd = mauBongVongNgoai->dd;
   hoaTietBongVong.mauBongVongNgoai.p = mauBongVongNgoai->p;
   
   
   
   return hoaTietBongVong;
}

Mau hoaTietBongVong( Vecto *viTri, HoaTietBongVong *hoaTietBongVong ) {
   
   // ---- vị trí tương đối tâm vất thể, quanh trục z
   Mau mau;
   float cachBinh = viTri->x*viTri->x + viTri->z*viTri->z;
   if( cachBinh < hoaTietBongVong->banKinhBinhNoi ) {
      mau = hoaTietBongVong->mauNen;
   }
   else if( cachBinh < hoaTietBongVong->banKinhBinhNgoai ) {
      // ---- tính màu, suy nội không bậc một nhe
      float tiSo = (cachBinh - hoaTietBongVong->banKinhBinhNoi)/(hoaTietBongVong->banKinhBinhNgoai - hoaTietBongVong->banKinhBinhNoi);
      float nghichTiSo = 1.0f - tiSo;
      mau.d = hoaTietBongVong->mauNoi.d*nghichTiSo + hoaTietBongVong->mauNgoai.d*tiSo;
      mau.l = hoaTietBongVong->mauNoi.l*nghichTiSo + hoaTietBongVong->mauNgoai.l*tiSo;
      mau.x = hoaTietBongVong->mauNoi.x*nghichTiSo + hoaTietBongVong->mauNgoai.x*tiSo;
      mau.dd = hoaTietBongVong->mauNoi.dd*nghichTiSo + hoaTietBongVong->mauNgoai.dd*tiSo;
      mau.p = hoaTietBongVong->mauNoi.p*nghichTiSo + hoaTietBongVong->mauNgoai.p*tiSo;
   }
   else if( cachBinh < hoaTietBongVong->banKinhBinhVanh ) {
      // ---- màu vành
      mau = hoaTietBongVong->mauVanh;
   }
   else {
      // ---- tính bán kính tương đối tâm vật thể
      float banKinh = sqrtf( viTri->x*viTri->x + viTri->z*viTri->z );
      // ---- tính góc
      float goc = 0.0f;
      if( banKinh != 0.0f )
         goc = atan( viTri->z/viTri->x );
      
      // ---- giữ 0 ≤ góc ≤ 2π
      if( viTri->x < 0.0f )
         goc += 3.141592f;
      else
         if( viTri->z < 0.0f )
            goc += 6.283184f;

      // ---- tính tâm của bông vòng tròn cho phần bông vòng này
      float gocTam = (floorf(goc/hoaTietBongVong->gocBongVong) + 0.5f)*hoaTietBongVong->gocBongVong;
//      printf( "goc %5.3f   gocTam %5.3f\n", goc, gocTam );
      float tamBongVongX = hoaTietBongVong->banKinhVanh*cosf( gocTam );
      float tamBongVongZ = hoaTietBongVong->banKinhVanh*sinf( gocTam );
//      printf( "tamBongVong %5.3f %5.3f\n", tamBongVongX, tamBongVongZ );
      // ---- vị trí tương đối với tâm bông vòng tròn
      float tuongDoiX = viTri->x - tamBongVongX;
      float tuongDoiZ = viTri->z - tamBongVongZ;
//      printf( "tuongDoi %5.3f  %5.3f\n", tuongDoiX, tuongDoiZ );
      // ---- tính cách từ tâm bông vòng tròn
      float cachTuTamBinh = tuongDoiX*tuongDoiX + tuongDoiZ*tuongDoiZ;
//      printf( "cachTuTam %5.3f\n", cachTuTamBinh );
//      printf( "hoaTietBongVong->banKinhBinhBongVongNgoai %5.3f\n", hoaTietBongVong->banKinhBinhBongVongNgoai );

      if( cachTuTamBinh < hoaTietBongVong->banKinhBinhBongVongNoi )
         mau = hoaTietBongVong->mauNen;
      else if(  cachTuTamBinh < hoaTietBongVong->banKinhBinhBongVongNgoai ) {
         // ---- tính màu, suy nội không bậc một nhe
         float tiSo = (cachTuTamBinh - hoaTietBongVong->banKinhBinhBongVongNoi)/(hoaTietBongVong->banKinhBinhBongVongNgoai - hoaTietBongVong->banKinhBinhBongVongNoi);
         float nghichTiSo = 1.0f - tiSo;
         mau.d = hoaTietBongVong->mauNoi.d*nghichTiSo + hoaTietBongVong->mauNgoai.d*tiSo;
         mau.l = hoaTietBongVong->mauNoi.l*nghichTiSo + hoaTietBongVong->mauNgoai.l*tiSo;
         mau.x = hoaTietBongVong->mauNoi.x*nghichTiSo + hoaTietBongVong->mauNgoai.x*tiSo;
         mau.dd = hoaTietBongVong->mauNoi.dd*nghichTiSo + hoaTietBongVong->mauNgoai.dd*tiSo;
         mau.p = hoaTietBongVong->mauNoi.p*nghichTiSo + hoaTietBongVong->mauNgoai.p*tiSo;
      }
      else {
         mau = hoaTietBongVong->mauNen;
      }
//      exit(0);
   }
      
   return mau;
}

#pragma mark ---- Họa Tiết Bầu Trời
HoaTietBauTroi datHoaTietBauTroi( Mau *mauDinhTroi, Mau *mauChanTroiTay, Mau *mauChanTroiDong, float gocXoayChanTroi ) {
   
   HoaTietBauTroi hoaTietBauTroi;
   hoaTietBauTroi.mauDinhTroi.d = mauDinhTroi->d;
   hoaTietBauTroi.mauDinhTroi.l = mauDinhTroi->l;
   hoaTietBauTroi.mauDinhTroi.x = mauDinhTroi->x;
   hoaTietBauTroi.mauDinhTroi.dd = mauDinhTroi->dd;
   hoaTietBauTroi.mauDinhTroi.p = mauDinhTroi->p;
   
   hoaTietBauTroi.mauChanTroiTay.d = mauChanTroiTay->d;
   hoaTietBauTroi.mauChanTroiTay.l = mauChanTroiTay->l;
   hoaTietBauTroi.mauChanTroiTay.x = mauChanTroiTay->x;
   hoaTietBauTroi.mauChanTroiTay.dd = mauChanTroiTay->dd;
   hoaTietBauTroi.mauChanTroiTay.p = mauChanTroiTay->p;
   
   hoaTietBauTroi.mauChanTroiDong.d = mauChanTroiDong->d;
   hoaTietBauTroi.mauChanTroiDong.l = mauChanTroiDong->l;
   hoaTietBauTroi.mauChanTroiDong.x = mauChanTroiDong->x;
   hoaTietBauTroi.mauChanTroiDong.dd = mauChanTroiDong->dd;
   hoaTietBauTroi.mauChanTroiDong.p = mauChanTroiDong->p;
   
   hoaTietBauTroi.gocXoayChanTroi = gocXoayChanTroi;
   
   return hoaTietBauTroi;
}

// tính màu theo hướng
//          -----
//        -       -
//      /           \
//     /             \
// TÂY +-π----+----0-+ ĐÔNG
//     \             /
//      \           /
//        -       -
//          -----
//
Mau hoaTietBauTroi( Vecto huongTia, HoaTietBauTroi *hoaTietBauTroi ) {
   
   donViHoa( &huongTia );
   
   float banKinh = sqrtf( huongTia.x*huongTia.x + huongTia.z*huongTia.z );
   
   // ---- tính góc
   float goc = 0.0f;
   if( banKinh != 0.0f )
      goc = atan( huongTia.z/huongTia.x );
   
   // ---- giữ 0 ≤ góc ≤ 2π
   if( huongTia.x < 0.0f )
      goc += 3.141592f;
   else
      if( huongTia.z < 0.0f )
         goc += 6.283184f;

   // ---- phạm vi 0 ≤ 2
   goc /= 3.141592f;
   
   // ---- tính màu chân trời
   Mau mauChanTroi;
   if( goc < 1.0f ) {
      float nghichGoc = 1.0f - goc;
      mauChanTroi.d = nghichGoc*hoaTietBauTroi->mauChanTroiDong.d + goc*hoaTietBauTroi->mauChanTroiTay.d;
      mauChanTroi.l = nghichGoc*hoaTietBauTroi->mauChanTroiDong.l + goc*hoaTietBauTroi->mauChanTroiTay.l;
      mauChanTroi.x = nghichGoc*hoaTietBauTroi->mauChanTroiDong.x + goc*hoaTietBauTroi->mauChanTroiTay.x;
      mauChanTroi.dd = nghichGoc*hoaTietBauTroi->mauChanTroiDong.dd + goc*hoaTietBauTroi->mauChanTroiTay.dd;
      mauChanTroi.p = nghichGoc*hoaTietBauTroi->mauChanTroiDong.p + goc*hoaTietBauTroi->mauChanTroiTay.p;
   }
   else {
      goc -= 1.0f;
      float nghichGoc = 1.0f - goc;
      mauChanTroi.d = nghichGoc*hoaTietBauTroi->mauChanTroiDong.d + goc*hoaTietBauTroi->mauChanTroiTay.d;
      mauChanTroi.l = nghichGoc*hoaTietBauTroi->mauChanTroiDong.l + goc*hoaTietBauTroi->mauChanTroiTay.l;
      mauChanTroi.x = nghichGoc*hoaTietBauTroi->mauChanTroiDong.x + goc*hoaTietBauTroi->mauChanTroiTay.x;
      mauChanTroi.dd = nghichGoc*hoaTietBauTroi->mauChanTroiDong.dd + goc*hoaTietBauTroi->mauChanTroiTay.dd;
      mauChanTroi.p = nghichGoc*hoaTietBauTroi->mauChanTroiDong.p + goc*hoaTietBauTroi->mauChanTroiTay.p;
   }
   
   Mau mauTroi;
   // ---- màu chân trời cho hướng dưới chên trời
   if( huongTia.y < 0.0f ) {
      mauTroi.d = mauChanTroi.d;
      mauTroi.l = mauChanTroi.l;
      mauTroi.x = mauChanTroi.x;
      mauTroi.dd = mauChanTroi.dd;
      mauTroi.p = mauChanTroi.p;
   }
   else {   // bầu trời
      float tiSoChanTroi = 1.0f - huongTia.y;
      tiSoChanTroi = powf( tiSoChanTroi, 8.0f );
      float nghichTiSoChanTroi = 1.0f - tiSoChanTroi;
      mauTroi.d = tiSoChanTroi*mauChanTroi.d + nghichTiSoChanTroi*hoaTietBauTroi->mauDinhTroi.d;
      mauTroi.l = tiSoChanTroi*mauChanTroi.l + nghichTiSoChanTroi*hoaTietBauTroi->mauDinhTroi.l;
      mauTroi.x = tiSoChanTroi*mauChanTroi.x + nghichTiSoChanTroi*hoaTietBauTroi->mauDinhTroi.x;
      mauTroi.dd = tiSoChanTroi*mauChanTroi.dd + nghichTiSoChanTroi*hoaTietBauTroi->mauDinhTroi.dd;
      mauTroi.p = tiSoChanTroi*mauChanTroi.p + nghichTiSoChanTroi*hoaTietBauTroi->mauDinhTroi.p;
   }

   return mauTroi;
}

#pragma mark ---- Họa Tiệt Ngẫu Nhiên
void chonVaToHoatTietNgauNhien( unsigned char *soHoaTiet, HoaTiet *hoaTiet ) {
   // ---- chọn họa tiệt
   *soHoaTiet = rand() % 9;

   if( *soHoaTiet == kHOA_TIET__CA_RO ) {
      Mau mau0;
      Mau mau1;
      mau0.d = rand()/kSO_NGUYEN_TOI_DA;
      mau0.l = rand()/kSO_NGUYEN_TOI_DA;
      mau0.x = rand()/kSO_NGUYEN_TOI_DA;
      mau0.dd = 1.0f;
      mau0.p = 0.1f;
      mau1.d = rand()/kSO_NGUYEN_TOI_DA;
      mau1.l = rand()/kSO_NGUYEN_TOI_DA;
      mau1.x = rand()/kSO_NGUYEN_TOI_DA;
      mau1.dd = 1.0f;
      mau1.p = 0.1f;
      float coKich = rand()/kSO_NGUYEN_TOI_DA + 0.3f;
      (*hoaTiet).hoaTietCaRo = datHoaTietCaRo( &mau0, &mau0, coKich, coKich, coKich );
   }
   else if( *soHoaTiet == kHOA_TIET__VONG_TRON ) {
      Mau mau0;
      Mau mau1;
      mau0.d = rand()/kSO_NGUYEN_TOI_DA;
      mau0.l = rand()/kSO_NGUYEN_TOI_DA;
      mau0.x = rand()/kSO_NGUYEN_TOI_DA;
      mau0.dd = 1.0f;
      mau0.p = 0.1f;
      mau1.d = rand()/kSO_NGUYEN_TOI_DA;
      mau1.l = rand()/kSO_NGUYEN_TOI_DA;
      mau1.x = rand()/kSO_NGUYEN_TOI_DA;
      mau1.dd = 1.0f;
      mau1.p = 0.1f;
      float coKich = rand()/kSO_NGUYEN_TOI_DA + 0.3f;
      (*hoaTiet).hoaTietVongTron = datHoaTietVongTron( &mau0, &mau1, coKich );
   }
   else if( *soHoaTiet == kHOA_TIET__OC_XOAY ) {
      Mau mau0;
      Mau mau1;
      mau0.d = rand()/kSO_NGUYEN_TOI_DA;
      mau0.l = rand()/kSO_NGUYEN_TOI_DA;
      mau0.x = rand()/kSO_NGUYEN_TOI_DA;
      mau0.dd = 1.0f;
      mau0.p = 0.1f;
      mau1.d = rand()/kSO_NGUYEN_TOI_DA;
      mau1.l = rand()/kSO_NGUYEN_TOI_DA;
      mau1.x = rand()/kSO_NGUYEN_TOI_DA;
      mau1.dd = 1.0f;
      mau1.p = 0.1f;
      float beRongNet = 0.3f*rand()/kSO_NGUYEN_TOI_DA + 0.2f;
      float phongTo = rand()/kSO_NGUYEN_TOI_DA + 1.0f;
      (*hoaTiet).hoaTietOcXoay = datHoaTietOcXoay( &mau0, &mau1, beRongNet, phongTo );
   }
   else if( *soHoaTiet == kHOA_TIET__GAN ) {
      Mau mauTren;
      Mau mauDuoi;
      mauTren.d = rand()/kSO_NGUYEN_TOI_DA;
      mauTren.l = rand()/kSO_NGUYEN_TOI_DA;
      mauTren.x = rand()/kSO_NGUYEN_TOI_DA;
      mauTren.dd = 1.0f;
      mauTren.p = 0.1f;
      mauDuoi.d = rand()/kSO_NGUYEN_TOI_DA;
      mauDuoi.l = rand()/kSO_NGUYEN_TOI_DA;
      mauDuoi.x = rand()/kSO_NGUYEN_TOI_DA;
      mauDuoi.dd = 1.0f;
      mauDuoi.p = 0.1f;
      float beCaoTren = 0.2f + 0.5f*rand()/kSO_NGUYEN_TOI_DA;
      float beCaoDuoi = 0.2f + 0.5f*rand()/kSO_NGUYEN_TOI_DA;
      (*hoaTiet).hoaTietGan = datHoaTietGan( &mauTren, &mauDuoi, beCaoTren, beCaoDuoi );
   }
   else if( *soHoaTiet == kHOA_TIET__CHAM_BI ) {
      Mau mauNen;
      Mau mauThap;
      Mau mauCao;
      mauNen.d = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.l = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.x = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.dd = 1.0f;
      mauNen.p = 0.1f;
      mauThap.d = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.l = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.x = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.dd = 1.0f;
      mauThap.p = 0.1f;
      mauCao.d = 1.0f;   mauCao.l = 1.0f;    mauCao.x = 1.0f;   mauCao.dd = 1.0f;   mauCao.p = 0.1f;
      (*hoaTiet).hoaTietChamBi = datHoaTietChamBi( &mauNen, &mauThap, &mauCao );
   }
   else if( *soHoaTiet == kHOA_TIET__QUAN ) {
      Mau mauNen;
      Mau mauThap;
      Mau mauCao;
      mauNen.d = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.l = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.x = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.dd = 1.0f;
      mauNen.p = 0.1f;
      mauThap.d = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.l = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.x = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.dd = 1.0f;
      mauThap.p = 0.1f;
      mauCao.d = 1.0f;   mauCao.l = 1.0f;    mauCao.x = 1.0f;   mauCao.dd = 1.0f;   mauCao.p = 0.1f;
      (*hoaTiet).hoaTietQuan = datHoaTietQuan( &mauNen, &mauThap, &mauCao );
   }
   else if( *soHoaTiet == kHOA_TIET__QUAN_XOAY ) {
      Mau mauNen;
      Mau mauOc0;
      Mau mauOc1;
      Mau mauOc2;
      mauNen.d = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.l = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.x = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.dd = 1.0f;
      mauNen.p = 0.1f;
      mauOc0.d = rand()/kSO_NGUYEN_TOI_DA;
      mauOc0.l = rand()/kSO_NGUYEN_TOI_DA;
      mauOc0.x = rand()/kSO_NGUYEN_TOI_DA;
      mauOc0.dd = 1.0f;
      mauOc0.p = 0.1f;
      mauOc1.d = rand()/kSO_NGUYEN_TOI_DA;
      mauOc1.l = rand()/kSO_NGUYEN_TOI_DA;
      mauOc1.x = rand()/kSO_NGUYEN_TOI_DA;
      mauOc1.dd = 1.0f;
      mauOc1.p = 0.1f;
      mauOc2.d = rand()/kSO_NGUYEN_TOI_DA;
      mauOc2.l = rand()/kSO_NGUYEN_TOI_DA;
      mauOc2.x = rand()/kSO_NGUYEN_TOI_DA;
      mauOc2.dd = 1.0f;
      mauOc2.p = 0.1f;
      (*hoaTiet).hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen, &mauOc0, &mauOc1, &mauOc2, 0.05f, 0.08f, 0.15f, 0.0f, 0.165f, 16 );
   }
   else if( *soHoaTiet == kHOA_TIET__DI_HUONG ){  //    if( *soHoaTiet == kHOA_TIET__KHONG ) {
      Mau mauRanh;
      mauRanh.d = rand()/kSO_NGUYEN_TOI_DA;
      mauRanh.l = rand()/kSO_NGUYEN_TOI_DA;
      mauRanh.x = rand()/kSO_NGUYEN_TOI_DA;
      mauRanh.dd = 1.0f;
      mauRanh.p = 0.1f;
      Mau mauTam;
      mauTam.d = rand()/kSO_NGUYEN_TOI_DA;
      mauTam.l = rand()/kSO_NGUYEN_TOI_DA;
      mauTam.x = rand()/kSO_NGUYEN_TOI_DA;
      mauTam.dd = 1.0f;
      mauTam.p = 0.1f;
      (*hoaTiet).hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   }
   else if( *soHoaTiet == kHOA_TIET__NGOI_SAO_CAU ) {
      Mau mauNen;
      Mau mauThap;
      Mau mauCao;
      mauNen.d = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.l = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.x = rand()/kSO_NGUYEN_TOI_DA;
      mauNen.dd = 1.0f;
      mauNen.p = 0.1f;
      mauThap.d = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.l = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.x = rand()/kSO_NGUYEN_TOI_DA;
      mauThap.dd = 1.0f;
      mauThap.p = 0.1f;
      mauCao.d = 1.0f;   mauCao.l = 1.0f;    mauCao.x = 1.0f;   mauCao.dd = 1.0f;   mauCao.p = 0.1f;
      float banKinhNoi = 0.1f + 0.05f*rand()/kSO_NGUYEN_TOI_DA;
      float banKinhNgoai = 0.15 + 0.2f*rand()/kSO_NGUYEN_TOI_DA;
      float chenhLech = rand()/kSO_NGUYEN_TOI_DA;
      unsigned char soNan = 3 + rand() & 0x07;
//      printf( "chonHoaTietngauNhien: banKinh %5.3f %5.3f chenhLech %5.3f soNan %d\n", banKinhNoi, banKinhNgoai, chenhLech, soNan );
      (*hoaTiet).hoaTietNgoiSaoCau = datHoaTietNgoiSaoCau( &mauNen, &mauThap, &mauCao, banKinhNoi, banKinhNgoai, chenhLech, soNan );
   }
   else {  //    if( *soHoaTiet == kHOA_TIET__KHONG ) {
      Mau mau;
      mau.d = rand()/kSO_NGUYEN_TOI_DA;
      mau.l = rand()/kSO_NGUYEN_TOI_DA;
      mau.x = rand()/kSO_NGUYEN_TOI_DA;
      mau.dd = 1.0f;
      mau.p = 0.1f;
      (*hoaTiet).hoaTietKhong = datHoaTietKhong( &mau );
   }
}

#pragma mark ---- Biến Hóa (Ma Trận)
void datDonVi( float *maTran ) {
   
   maTran[0] = 1.0f;
   maTran[1] = 0.0f;
   maTran[2] = 0.0f;
   maTran[3] = 0.0f;
   
   maTran[4] = 0.0f;
   maTran[5] = 1.0f;
   maTran[6] = 0.0f;
   maTran[7] = 0.0f;
   
   maTran[8] = 0.0f;
   maTran[9] = 0.0f;
   maTran[10] = 1.0f;
   maTran[11] = 0.0f;
   
   maTran[12] = 0.0f;
   maTran[13] = 0.0f;
   maTran[14] = 0.0f;
   maTran[15] = 1.0f;
};

void datPhongTo( float *maTran, float phongToX, float phongToY, float phongToZ ) {
   
   maTran[0] = phongToX;
   maTran[1] = 0.0f;
   maTran[2] = 0.0f;
   maTran[3] = 0.0f;
   
   maTran[4] = 0.0f;
   maTran[5] = phongToY;
   maTran[6] = 0.0f;
   maTran[7] = 0.0f;
   
   maTran[8] = 0.0f;
   maTran[9] = 0.0f;
   maTran[10] = phongToZ;
   maTran[11] = 0.0f;
   
   maTran[12] = 0.0f;
   maTran[13] = 0.0f;
   maTran[14] = 0.0f;
   maTran[15] = 1.0f;
};

void datDich( float *maTran, float x, float y, float z ) {
   
   maTran[0] = 1.0f;
   maTran[1] = 0.0f;
   maTran[2] = 0.0f;
   maTran[3] = 0.0f;
   
   maTran[4] = 0.0f;
   maTran[5] = 1.0f;
   maTran[6] = 0.0f;
   maTran[7] = 0.0f;
   
   maTran[8] = 0.0f;
   maTran[9] = 0.0f;
   maTran[10] = 1.0f;
   maTran[11] = 0.0f;
   
   maTran[12] = x;
   maTran[13] = y;
   maTran[14] = z;
   maTran[15] = 1.0f;
};

void latMaTran4x4( float *maTran ) {
   float giaTri;
   giaTri = maTran[1];
   maTran[1] = maTran[4];
   maTran[4] = giaTri;
   // ----
   giaTri = maTran[2];
   maTran[2] = maTran[8];
   maTran[8] = giaTri;
   // ----
   giaTri = maTran[3];
   maTran[3] = maTran[12];
   maTran[12] = giaTri;
   // ----
   giaTri = maTran[6];
   maTran[6] = maTran[9];
   maTran[9] = giaTri;
   // ----
   giaTri = maTran[7];
   maTran[7] = maTran[13];
   maTran[13] = giaTri;
   // ----
   giaTri = maTran[11];
   maTran[11] = maTran[14];
   maTran[14] = giaTri;
}

Vecto nhanVectoVoiMaTran3x3( Vecto *vecto, float *maTran ) {
   
   Vecto ketQua;
   ketQua.x = vecto->x*maTran[0] + vecto->y*maTran[4] + vecto->z*maTran[8];
   ketQua.y = vecto->x*maTran[1] + vecto->y*maTran[5] + vecto->z*maTran[9];
   ketQua.z = vecto->x*maTran[2] + vecto->y*maTran[6] + vecto->z*maTran[10];
   
   return ketQua;
}

Vecto nhanVectoVoiMaTran4x4( Vecto *vecto, float *maTran ) {

   Vecto ketQua;
   ketQua.x = vecto->x*maTran[0] + vecto->y*maTran[4] + vecto->z*maTran[8] + maTran[12];
   ketQua.y = vecto->x*maTran[1] + vecto->y*maTran[5] + vecto->z*maTran[9] + maTran[13];
   ketQua.z = vecto->x*maTran[2] + vecto->y*maTran[6] + vecto->z*maTran[10] + maTran[14];

   return ketQua;
}

void nhanMaTranVoiMaTran( float *maTranKetQua, float *maTran1, float *maTran2 ) {
   
   // ---- giũ kết quả riêng cho có thể bỏ kết qủa vào maTran1 hay maTran2
   float ketQua00 = maTran1[0]*maTran2[0] + maTran1[1]*maTran2[4] + maTran1[2]*maTran2[8] + maTran1[3]*maTran2[12];
   float ketQua01 = maTran1[0]*maTran2[1] + maTran1[1]*maTran2[5] + maTran1[2]*maTran2[9] + maTran1[3]*maTran2[13];
   float ketQua02 = maTran1[0]*maTran2[2] + maTran1[1]*maTran2[6] + maTran1[2]*maTran2[10] + maTran1[3]*maTran2[14];
   float ketQua03 = maTran1[0]*maTran2[3] + maTran1[1]*maTran2[7] + maTran1[2]*maTran2[11] + maTran1[3]*maTran2[15];
   
   float ketQua10 = maTran1[4]*maTran2[0] + maTran1[5]*maTran2[4] + maTran1[6]*maTran2[8] + maTran1[7]*maTran2[12];
   float ketQua11 = maTran1[4]*maTran2[1] + maTran1[5]*maTran2[5] + maTran1[6]*maTran2[9] + maTran1[7]*maTran2[13];
   float ketQua12 = maTran1[4]*maTran2[2] + maTran1[5]*maTran2[6] + maTran1[6]*maTran2[10] + maTran1[7]*maTran2[14];
   float ketQua13 = maTran1[4]*maTran2[3] + maTran1[5]*maTran2[7] + maTran1[6]*maTran2[11] + maTran1[7]*maTran2[15];
   
   float ketQua20 = maTran1[8]*maTran2[0] + maTran1[9]*maTran2[4] + maTran1[10]*maTran2[8] + maTran1[11]*maTran2[12];
   float ketQua21 = maTran1[8]*maTran2[1] + maTran1[9]*maTran2[5] + maTran1[10]*maTran2[9] + maTran1[11]*maTran2[13];
   float ketQua22 = maTran1[8]*maTran2[2] + maTran1[9]*maTran2[6] + maTran1[10]*maTran2[10] + maTran1[11]*maTran2[14];
   float ketQua23 = maTran1[8]*maTran2[3] + maTran1[9]*maTran2[7] + maTran1[10]*maTran2[11] + maTran1[11]*maTran2[15];
   
   float ketQua30 = maTran1[12]*maTran2[0] + maTran1[13]*maTran2[4] + maTran1[14]*maTran2[8] + maTran1[15]*maTran2[12];
   float ketQua31 = maTran1[12]*maTran2[1] + maTran1[13]*maTran2[5] + maTran1[14]*maTran2[9] + maTran1[15]*maTran2[13];
   float ketQua32 = maTran1[12]*maTran2[2] + maTran1[13]*maTran2[6] + maTran1[14]*maTran2[10] + maTran1[15]*maTran2[14];
   float ketQua33 = maTran1[12]*maTran2[3] + maTran1[13]*maTran2[7] + maTran1[14]*maTran2[11] + maTran1[15]*maTran2[15];
   
   maTranKetQua[0] = ketQua00;
   maTranKetQua[1] = ketQua01;
   maTranKetQua[2] = ketQua02;
   maTranKetQua[3] = ketQua03;
   
   maTranKetQua[4] = ketQua10;
   maTranKetQua[5] = ketQua11;
   maTranKetQua[6] = ketQua12;
   maTranKetQua[7] = ketQua13;
   
   maTranKetQua[8] = ketQua20;
   maTranKetQua[9] = ketQua21;
   maTranKetQua[10] = ketQua22;
   maTranKetQua[11] = ketQua23;
   
   maTranKetQua[12] = ketQua30;
   maTranKetQua[13] = ketQua31;
   maTranKetQua[14] = ketQua32;
   maTranKetQua[15] = ketQua33;
}

#pragma mark ---- Định Hướng Bằng Vectơ Nhìn
// ---- này giả sự không nghiêng nhín thẳng lên hay thẳng xuống, khác (0; ±1; 0)
void dinhHuongMaTranBangVectoNhin( float *maTran, Vecto *vectoNhin ) {
   
   // ---- vectơ nhìn làm trục z địa phương
   donViHoa( vectoNhin );
   // ---- hướng vô tích vectơ nhìn với vectơ trục y cho tính trúc x đia phuong
   Vecto trucY;
   trucY.x = 0.0f;
   trucY.y = 1.0f;
   trucY.z = 0.0f;
   Vecto trucX_diaPhuong = tichCoHuong( &trucY, vectoNhin );
   // ---- hướng vô tích vectơ nhìn với trục vô tích
   Vecto trucY_diaPhuong = tichCoHuong( vectoNhin, &trucX_diaPhuong );

   // ---- giỡ kết qủa
   maTran[0] = trucX_diaPhuong.x;
   maTran[1] = trucX_diaPhuong.y;
   maTran[2] = trucX_diaPhuong.z;
   maTran[3] = 0.0f;

   maTran[4] = trucY_diaPhuong.x;
   maTran[5] = trucY_diaPhuong.y;
   maTran[6] = trucY_diaPhuong.z;
   maTran[7] = 0.0f;

   maTran[8] = vectoNhin->x;
   maTran[9] = vectoNhin->y;
   maTran[10] = vectoNhin->z;
   maTran[11] = 0.0f;
   
   maTran[12] = 0.0f;
   maTran[13] = 0.0f;
   maTran[14] = 0.0f;
   maTran[15] = 1.0f;
}

#pragma mark ---- Quaternion
Quaternion datQuaternionTuVectoVaGocQuay( Vecto *vecto, float gocQuay ) {
   
   donViHoa( vecto );
   // ---- nữa góc quay (âm cho quay theo qũy tắc tay phải)
   float nuaGocQuay = -gocQuay*0.5f;  // /banKinh;
   float sinNuaGocQuay = sinf( nuaGocQuay );
   // ---- tính quaternion
   Quaternion quaternion;
   quaternion.w = cosf( nuaGocQuay );
   quaternion.x = vecto->x*sinNuaGocQuay;
   quaternion.y = vecto->y*sinNuaGocQuay;
   quaternion.z = vecto->z*sinNuaGocQuay;

   return quaternion;
}

void quaternionQuaMaTran( Quaternion *quaternion, float *maTran ) {
   
   float doLonBinhPhong = quaternion->w*quaternion->w + quaternion->x*quaternion->x + quaternion->y*quaternion->y + quaternion->z*quaternion->z;
	maTran[0] = doLonBinhPhong - 2.0f*(quaternion->y*quaternion->y + quaternion->z*quaternion->z);
	maTran[1] = 2.0f*(quaternion->x*quaternion->y - quaternion->w*quaternion->z);
	maTran[2] = 2.0f*(quaternion->x*quaternion->z + quaternion->w*quaternion->y);
   maTran[3] = 0.0f;
   
   maTran[4] = 2.0f*(quaternion->x*quaternion->y + quaternion->w*quaternion->z);
   maTran[5] = doLonBinhPhong - 2.0f*(quaternion->x*quaternion->x + quaternion->z*quaternion->z);
   maTran[6] = 2.0f*(quaternion->y*quaternion->z - quaternion->w*quaternion->x);
   maTran[7] = 0.0f;
   
   maTran[8] = 2.0f*(quaternion->x*quaternion->z - quaternion->w*quaternion->y);
	maTran[9] = 2.0f*(quaternion->y*quaternion->z + quaternion->w*quaternion->x);
	maTran[10] = doLonBinhPhong - 2.0f*(quaternion->x*quaternion->x + quaternion->y*quaternion->y);
   maTran[11] = 0.0f;
   
   maTran[12] = 0.0f;
   maTran[13] = 0.0f;
   maTran[14] = 0.0f;
   maTran[15] = 1.0f;
   
   // đơn vị hóa maTran
	maTran[0] /= doLonBinhPhong;
	maTran[1] /= doLonBinhPhong;
	maTran[2] /= doLonBinhPhong;
   
	maTran[4] /= doLonBinhPhong;
	maTran[5] /= doLonBinhPhong;
	maTran[6] /= doLonBinhPhong;
   
	maTran[8] /= doLonBinhPhong;
	maTran[9] /= doLonBinhPhong;
	maTran[10] /= doLonBinhPhong;
}

Quaternion nhanQuaternionVoiQuaternion( Quaternion *quaternion0, Quaternion *quaternion1 ) {
   
   Quaternion quaternionKetQua;
   // ---- nhân quaternion0 * quaternion1
   quaternionKetQua.w = quaternion0->w*quaternion1->w - quaternion0->x*quaternion1->x - quaternion0->y*quaternion1->y - quaternion0->z*quaternion1->z;
   quaternionKetQua.x = quaternion0->w*quaternion1->x + quaternion0->x*quaternion1->w + quaternion0->y*quaternion1->z - quaternion0->z*quaternion1->y;
   quaternionKetQua.y = quaternion0->w*quaternion1->y - quaternion0->x*quaternion1->z + quaternion0->y*quaternion1->w + quaternion0->z*quaternion1->x;
	quaternionKetQua.z = quaternion0->w*quaternion1->z + quaternion0->x*quaternion1->y - quaternion0->y*quaternion1->x + quaternion0->z*quaternion1->w;

   return quaternionKetQua;
}

#pragma mark ---- Đăt Biến Hóa Và Gồm Biến Hóa
void datBienHoaChoVat( VatThe *vatThe, Vecto *phongTo, Quaternion *xoay, Vecto *dich ) {
   // ---- phóng to
   datPhongTo( vatThe->phongTo, phongTo->x, phongTo->y, phongTo->z );
   datPhongTo( vatThe->nghichPhongTo, 1.0f/phongTo->x, 1.0f/phongTo->y, 1.0f/phongTo->z );
   // ---- chưa làm xoay
   vatThe->quaternion.w = xoay->w;
   vatThe->quaternion.x = xoay->x;
   vatThe->quaternion.y = xoay->y;
   vatThe->quaternion.z = xoay->z;

   quaternionQuaMaTran( xoay, vatThe->xoay );
   quaternionQuaMaTran( xoay, vatThe->nghichXoay );   //  nên chép này, không tính hai lần
   latMaTran4x4( vatThe->nghichXoay );
   // ---- dịch
   datDich( vatThe->dich, dich->x, dich->y, dich->z );
   datDich( vatThe->nghichDich, -dich->x, -dich->y, -dich->z );
   gomBienHoaChoVat( vatThe );
}


void datXoayChoVat( VatThe *vatThe, Quaternion *quaternion ) {

   vatThe->quaternion.w = quaternion->w;
   vatThe->quaternion.x = quaternion->x;
   vatThe->quaternion.y = quaternion->y;
   vatThe->quaternion.z = quaternion->z;

   // ---- chưa làm xoay
   quaternionQuaMaTran( quaternion, vatThe->xoay );
   quaternionQuaMaTran( quaternion, vatThe->nghichXoay );   //  nên chép này, không tính hai lần
   latMaTran4x4( vatThe->nghichXoay );

   gomBienHoaChoVat( vatThe );

}


void gomBienHoaChoVat( VatThe *vatThe ) {
   
   // ==== biến hóa   // kèm thêm ở đang sau như này: [vectơ] * [phóng to] * [xoay] * [dịch]
   // ---- phóng to và xoay
   float maTranKetQua[16];
   nhanMaTranVoiMaTran( maTranKetQua, vatThe->phongTo, vatThe->xoay );
   // ---- phóng to và xoay và dịch
   nhanMaTranVoiMaTran( vatThe->bienHoa, maTranKetQua, vatThe->dich );
   
   // ---- nghịch biến hóa     // kèm nghịch: [vectơ] * [dịch]-1 *[xoay]-1 * [phóng to]-1
   nhanMaTranVoiMaTran( maTranKetQua, vatThe->nghichXoay, vatThe->nghichPhongTo );
   nhanMaTranVoiMaTran( vatThe->nghichBienHoa, vatThe->nghichDich, maTranKetQua );
   // ---- đặc biệt cho nghịch biến hóa cho pháp tuyến
   nhanMaTranVoiMaTran( vatThe->nghichBienHoaChoPhapTuyen, vatThe->nghichPhongTo, vatThe->xoay );
   
/*   printf( "\n-----vatThe->nghichXoay\n" );
   printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichXoay[0], vatThe->nghichXoay[1], vatThe->nghichXoay[2], vatThe->nghichXoay[3] );
   printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichXoay[4], vatThe->nghichXoay[5], vatThe->nghichXoay[6], vatThe->nghichXoay[7] );
   printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichXoay[8], vatThe->nghichXoay[9], vatThe->nghichXoay[10], vatThe->nghichXoay[11] );
   printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichXoay[12], vatThe->nghichXoay[13], vatThe->nghichXoay[14], vatThe->nghichXoay[15] );
   printf( "\n-----vatThe->nghichBienHoa\n" );
   printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichBienHoa[0], vatThe->nghichBienHoa[1], vatThe->nghichBienHoa[2], vatThe->nghichBienHoa[3] );
   printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichBienHoa[4], vatThe->nghichBienHoa[5], vatThe->nghichBienHoa[6], vatThe->nghichBienHoa[7] );
   printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichBienHoa[8], vatThe->nghichBienHoa[9], vatThe->nghichBienHoa[10], vatThe->nghichBienHoa[11] );
   printf( "%5.3f %5.3f %5.3f %5.3f\n", vatThe->nghichBienHoa[12], vatThe->nghichBienHoa[13], vatThe->nghichBienHoa[14], vatThe->nghichBienHoa[15] ); */
}

Vecto xoayVectoQuanhTruc( Vecto *vecto, Vecto *trucXoay, float gocXoay ) {

   // ---- tạo quaternion cho biến hóa
   Quaternion quaternionXoay = datQuaternionTuVectoVaGocQuay( trucXoay, gocXoay );
   float maTran[16];
   quaternionQuaMaTran( &quaternionXoay, maTran );
   
   return nhanVectoVoiMaTran3x3( vecto, maTran );
}


//#pragma mark ---- Xem Điểm
/* Quaternion tuDiemGocXemDiem( Vecto *diemGoc, Vecto *diemXem, float gocXoay ) {

   // ---- tính hướng nhìn
   Vecto huongNhin;
   huongNhin.x = diemXem->x - diemGoc->x;
   huongNhin.y = diemXem->y - diemGoc->y;
   huongNhin.z = diemXem->z - diemGoc->z;
   donViHoa( &huongNhin );
   // ---- tính quaternion
   Quaternion quaternion = datQuaternionTuVectoVaGocQuay( &huongNhin, gocXoay );
   
   return quaternion;
} */

#pragma mark ---- Di Chuyển
// số bức ảnh - số bức ảnh đang tính vị trí (tư đối với số bức ành đầu chu kỳ)
// số bức ảnh chu kỳ - số lượng chu kỳ cho một chu kỳ di chuyển
/*Vecto vanTocSin( Vecto *huong, float bienDo, float soBucAnh, float soBucAnhChuKy ) {
   // ---- tính góc xoay
   float gocXoay = 6.28318f*soBucAnh/soBucAnhChuKy;
   float doLon = bienDo*sinf( gocXoay );
   printf( "gocXoay %5.3f   doLon %5.3f   soBucAnh %5.3f\n", gocXoay, doLon, soBucAnh );
   // ---- vectơ di chuyển từ điểm gốc
   Vecto vanTocDiChuyen;
   vanTocDiChuyen.x = doLon*huong->x;
   vanTocDiChuyen.y = doLon*huong->y;
   vanTocDiChuyen.z = doLon*huong->z;

   return vanTocDiChuyen;
}*/

Vecto viTriSin( Vecto *viTriDau, Vecto *huong, float bienDo, float soBucAnh, float soBucAnhChuKy, float *tocDo ) {
   // ---- tính góc xoay
   float gocXoay = 6.28318f*soBucAnh/soBucAnhChuKy;
   float doLon = bienDo*(1.0f - cosf( gocXoay ));
   *tocDo = bienDo*6.28318f/soBucAnhChuKy*sinf( gocXoay );
//   printf( "gocXoay %5.3f   doLon %5.3f   soBucAnh %5.3f\n", gocXoay, doLon, soBucAnh );
   // ---- vectơ di chuyển từ điểm gốc
   Vecto viTri;
   viTri.x = viTriDau->x + doLon*huong->x;
   viTri.y = viTriDau->y + doLon*huong->y;
   viTri.z = viTriDau->z + doLon*huong->z;
   
   return viTri;
}

Vecto viTriSin2( Vecto *viTriDau, Vecto *huong, float bienDo, float soBucAnh, float soBucAnhChuKy, float *tocDo ) {
   // ---- tính góc xoay
   float gocXoay = 6.28318f*soBucAnh/soBucAnhChuKy;
   float doLon = bienDo*sinf( gocXoay );
   *tocDo = bienDo*6.28318f/soBucAnhChuKy*cosf( gocXoay );
//   printf( "gocXoay %5.3f   doLon %5.3f   soBucAnh %5.3f\n", gocXoay, doLon, soBucAnh );
   // ---- vectơ di chuyển từ điểm gốc
   Vecto viTri;
   viTri.x = viTriDau->x + doLon*huong->x;
   viTri.y = viTriDau->y + doLon*huong->y;
   viTri.z = viTriDau->z + doLon*huong->z;
   
   return viTri;
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

void docThamSoHoatHinh( int argc, char **argv, unsigned int *soHoatHinhDau, unsigned int *soHoatHinhCuoi ) {

   if( argc > 3 ) {
      sscanf( argv[2], "%u", soHoatHinhDau );
      sscanf( argv[3], "%u", soHoatHinhCuoi );
      // ---- kiểm tra soHoatHinhDau nhỏ soHoatHinhCuoi
      if( *soHoatHinhDau >= *soHoatHinhCuoi )
         *soHoatHinhCuoi = *soHoatHinhDau + 1;
   }
   else if ( argc > 2 ){
      sscanf( argv[2], "%u", soHoatHinhDau );
      *soHoatHinhCuoi = *soHoatHinhDau + 1;
   }
   else {
      *soHoatHinhDau = 0;
   }
}

void docThamCoKich( int argc, char **argv, unsigned int *beRong, unsigned int *beCao, float *coThuocDiemAnh ) {
   
   if( argc > 6 ) {
      sscanf( argv[4], "%u", beRong );
      sscanf( argv[5], "%u", beCao );
      sscanf( argv[6], "%f", coThuocDiemAnh );
      // ---- kiểm tra > 100
      if( *beRong == 0 )
         *beRong = 1;
      else if( *beRong > 8192 )
         *beRong = 8192;
      
      if( *beCao == 0 )
         *beCao = 1;
      else if( *beCao > 8192 )
         *beCao = 8192;
      
      if( *coThuocDiemAnh < 0.00001f )
         *coThuocDiemAnh = 0.00001f;
      else if( *coThuocDiemAnh > 5.0f )
         *coThuocDiemAnh = 5.0f;
   }
   else {
      *beRong = 200;
      *beCao = 100;
      *coThuocDiemAnh = 0.03f;
   }
   
}

#pragma mark ---- PHIM TRƯỜNG 0
#define kNHAN_VAT__VAI_CHANH           0       // nhân vật vai chánh
#define kNHAN_VAT__SAO_GAI             1       // nhân vật sao gai
#define kNHAN_VAT__TRAI_BANH_BI_GIET   2  // nhân vật sao gai
#define kNHAN_VAT__KIEN_LONG           3  // nhân vật kiến lồng
#define kNHAN_VAT__HAT_THUY_TINH_BAY   4  // nhân vật hạt thủy tinh bay
#define kNHAN_VAT__BAY_TRAI_BANH_DAU   5  // nhân vật bài trái banh đầu
#define kNHAN_VAT__BAY_TRAI_BANH_CUOI  6  // nhân vật bài trái banh cuối
#define kNHAN_VAT__BONG_BONG_DAU       7  // nhân vật bong bóng đầu
#define kNHAN_VAT__BONG_BONG_CUOI      8  // nhân vật bong bóng cuối
#define kNHAN_VAT__BAT_DIEN_XOAY_DAU   9  // nhân vật bong bóng đầu
#define kNHAN_VAT__BAT_DIEN_XOAY_CUOI 10  // nhân vật bong bóng cuối
#define kNHAN_VAT__COT_NHAY_DAU       11  // nhân vật hình trụ nhảy
#define kNHAN_VAT__COT_NHAY_CUOI      12  // nhân vật hình trụ nhảy

// ---- hằng số
#define kHAP_DAN -0.064f
#define kTOC_DO_TRAI_BANH_PHIM_TRUONG0 0.5f
#define kBAN_KINH__VONG_NOI_BINH  95.0f  // bán kính vòng nội binh cho trái banh quanh vòng xuyến   10.0
#define kBAN_KINH__VONG_NGOAI_BINH 190.0f   // bán kính vòng ngoại binh cho trái banh quanh vòng xuyến 14,14

void chuanBiMayQuayPhimVaMatTroiPhimTruong0();

// ---- vật thể không chuyển động
unsigned short datMatBien( VatThe *danhSachVat );
unsigned short datCanhGocBoBien( VatThe *danhSachVatThe );
unsigned short datCotNghiengSauLongGai( VatThe *danhSachVatThe );
unsigned short datDuongLo( VatThe *danhSachVat );
unsigned short datLanCanDuongLo( VatThe *danhSachVat );

unsigned short datThapVongXuyen( VatThe *danhSachVat );
unsigned short datDayCotQuangVongXuyen( VatThe *danhSachVat );
unsigned short datLongSaoGai( VatThe *danhSachVat );
unsigned short datVuonQuangDatLongSaoGai( VatThe *danhSachVat );
unsigned short datLanCanNgoaiQuanhLongSaoGai( VatThe *danhSachVat );
unsigned short datLanCanTrongQuanhLongSaoGai( VatThe *danhSachVat );
unsigned short datBonThapVuonSaoGai( VatThe *danhSachVat );
unsigned short datCacCotVuongLong( VatThe *danhSachVat );
unsigned short datKimTuThapThuyTinh( VatThe *danhSachVat );
unsigned short datDayHinhNonHuongX( VatThe *danhSachVat );
unsigned short datDayHinhNonHuongZ( VatThe *danhSachVat );

// ---- nhân vật (vật thể chuyển động)
unsigned short datKienLongSaoGai( VatThe *danhSachVat );
unsigned short datHinhNonBayQuanhThap( VatThe *danhSachVat );
unsigned short datKeThuSaoGai( VatThe *danhSachVat, Vecto viTri );
unsigned short datCacTraiBanhTrenLoHuongZ( VatThe *danhSachVat );
unsigned short datCacTraiBanhTrenLoHuongX( VatThe *danhSachVat );
unsigned short datVaiChanh( VatThe *danhSachVat, Vecto viTri );
unsigned short datTraiBanhBiGiet( VatThe *danhSachVat );
unsigned short datBongBong( VatThe *danhSachVat );
unsigned short datCacBatDienXoay( VatThe *danhSachVat );
unsigned short datCotNhay( VatThe *danhSachVat );

void moPhongTraiBanhGiaoThong( VatThe *danhSachVatThe, unsigned short soTraiBanhDau, unsigned short soTraiBanhCuoi, Vecto *viTriSaoGai );
void nangCapPhimTruong0_mayQuayPhim( PhimTruong *phimTruong );
void nangCapPhimTruong0_nhanVat( PhimTruong *phimTruong, unsigned short soHoatHinh );
void nangCapVaiChanh( VatThe *vaiChanh, unsigned short soHoatHinh );
void nangCapSaoGai( VatThe *saoGai, unsigned short soHoatHinh );
void nangCapTraiBanhBiGiet( VatThe *traiBanh, unsigned short soHoatHinh );
void nangCapKienLongSaoGai( VatThe *kienLongSaoGai, unsigned short soHoatHinh );
void nangCapHatBayQuanhVongXuyen( VatThe *vaiChanh, unsigned short soHoatHinh );
void nangCapBongBongBay( VatThe *danhSachBongBong, unsigned short soLuongVatThe, unsigned short soHoatHinh );
void nangCapCacBatDienXoay( VatThe *danhSachBatDien, unsigned short soLuongBatDien, unsigned short soHoatHinh );
void nangCapCotNhay( VatThe *danhSachBatDien, unsigned short soLuongBatDien, unsigned short soHoatHinh );

Vecto vanTocChoViTriChoPhimTruong0( Vecto *viTri, unsigned char duongVao, unsigned char duongRa, unsigned char *thaiTrang, Vecto *trucXoay );
Vecto vanTocNeSaoGaiRaDuongZ( Vecto *viTriTraiBanh, unsigned char *thaiTrang, Vecto *trucXoay, Vecto *viTriSaoGai );
Vecto vanTocNeSaoGaiTrongVongXuyen( Vecto *viTriTraiBanh, Vecto *trucXoay, float quangBinh );

Quaternion tinhXoayChoVatThe( VatThe *vatThe, Vecto trucXoay, float tocDo, float banKinh );

#define kVI_TRI__VAI_CHANH_DAU_X  1.5f
#define kVI_TRI__VAI_CHANH_DAU_Y 10.8f
#define kVI_TRI__VAI_CHANH_DAU_Z -7.5f

PhimTruong datPhimTruongSo0( unsigned int argc, char **argv ) {
   
   PhimTruong phimTruong;
   phimTruong.soNhoiToiDa = 5;

   unsigned int soHoatHinhDau = 0;
   unsigned int soHoatHinhCuoi = 1520;     // số bức ảnh cuối cho phim trường này

   docThamSoHoatHinh( argc, argv, &soHoatHinhDau, &soHoatHinhCuoi );
   if( soHoatHinhDau > 1519 )
      soHoatHinhDau = 1519;
   if( soHoatHinhCuoi > 1520 )     // số bức ảnh cuối cho phim trường này
      soHoatHinhCuoi = 1520;
   
   phimTruong.soHoatHinhDau = soHoatHinhDau;
   phimTruong.soHoatHinhHienTai = soHoatHinhDau;
   phimTruong.soHoatHinhCuoi = soHoatHinhCuoi;
   
   phimTruong.soLuongVatThe = 0;
   phimTruong.danhSachVatThe = malloc( kSO_LUONG_VAT_THE_TOI_DA*sizeof(VatThe) );

   // ---- chuẩn bị máy quay phim
   chuanBiMayQuayPhimVaMatTroiPhimTruong0( &phimTruong );
   Mau mauDinhTroi;
   mauDinhTroi.d = 0.0f;   mauDinhTroi.l = 0.0f;   mauDinhTroi.x = 0.5f;   mauDinhTroi.dd = 1.0f;
   Mau mauChanTroi;  // chỉ có một;
   mauChanTroi.d = 0.7f;  mauChanTroi.l = 0.7f;   mauChanTroi.x = 1.0f;    mauChanTroi.dd = 1.0f;
   phimTruong.hoaTietBauTroi = datHoaTietBauTroi( &mauDinhTroi, &mauChanTroi, &mauChanTroi, 0.0f );

   VatThe *danhSachVat = phimTruong.danhSachVatThe;

   // ---- cảnh
   phimTruong.soLuongVatThe += datMatBien( &(danhSachVat[phimTruong.soLuongVatThe]) );

   phimTruong.soLuongVatThe += datCanhGocBoBien( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datCotNghiengSauLongGai( &(danhSachVat[phimTruong.soLuongVatThe]) );

   phimTruong.soLuongVatThe += datDuongLo( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datLanCanDuongLo( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datThapVongXuyen( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datDayCotQuangVongXuyen( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- lồng sao gai
   phimTruong.soLuongVatThe += datLongSaoGai( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datVuonQuangDatLongSaoGai( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datLanCanNgoaiQuanhLongSaoGai( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datLanCanTrongQuanhLongSaoGai( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datBonThapVuonSaoGai( &(danhSachVat[phimTruong.soLuongVatThe]) );

   // ---- kim tư tháp
   phimTruong.soLuongVatThe += datKimTuThapThuyTinh( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- dãy hình nón
   phimTruong.soLuongVatThe += datDayHinhNonHuongX( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datDayHinhNonHuongZ( &(danhSachVat[phimTruong.soLuongVatThe]) );

   // ---- CÁC NHÂN VẬT
   // sao gai
   // Cái sẽ bị hư gì hệ thống tăng tốc độ kết xuất sẽ đổi thứ tự vật thề <---------
   phimTruong.nhanVat[kNHAN_VAT__SAO_GAI] = phimTruong.soLuongVatThe;
   Vecto viTri;
   viTri.x = -52.0f;      viTri.y = 27.7f;        viTri.z = -52.0f;
   phimTruong.soLuongVatThe += datKeThuSaoGai( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );
   // ---- kiến long
   phimTruong.nhanVat[kNHAN_VAT__KIEN_LONG] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datKienLongSaoGai( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- hình nón bay quanh tháp
   phimTruong.nhanVat[kNHAN_VAT__HAT_THUY_TINH_BAY] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datHinhNonBayQuanhThap( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- bày trái banh
   phimTruong.nhanVat[kNHAN_VAT__BAY_TRAI_BANH_DAU] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datCacTraiBanhTrenLoHuongZ( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datCacTraiBanhTrenLoHuongX( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.nhanVat[kNHAN_VAT__BAY_TRAI_BANH_CUOI] = phimTruong.soLuongVatThe;

   // ---- vai chánh
   viTri.x = kVI_TRI__VAI_CHANH_DAU_X;  viTri.y = kVI_TRI__VAI_CHANH_DAU_Y; viTri.z = kVI_TRI__VAI_CHANH_DAU_Z;
   phimTruong.nhanVat[kNHAN_VAT__VAI_CHANH] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datVaiChanh( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );
   
   // ---- trái bị giết
   phimTruong.nhanVat[kNHAN_VAT__TRAI_BANH_BI_GIET] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datTraiBanhBiGiet( &(danhSachVat[phimTruong.soLuongVatThe]) );

   // ---- các bong bóng
   phimTruong.nhanVat[kNHAN_VAT__BONG_BONG_DAU] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datBongBong( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.nhanVat[kNHAN_VAT__BONG_BONG_CUOI] = phimTruong.soLuongVatThe;
   
   // ---- bát diận xoay
   phimTruong.nhanVat[kNHAN_VAT__BAT_DIEN_XOAY_DAU] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datCacBatDienXoay( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.nhanVat[kNHAN_VAT__BAT_DIEN_XOAY_CUOI] = phimTruong.soLuongVatThe;
   
   // ---- cột nhẩ quanh vòng xuyên
   phimTruong.nhanVat[kNHAN_VAT__COT_NHAY_DAU] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datCotNhay( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.nhanVat[kNHAN_VAT__COT_NHAY_CUOI] = phimTruong.soLuongVatThe;

   // ---- nâng cấp mô phỏng cho các trái banh và nhân vật
   unsigned short soHoatHinh = 0;
   while ( soHoatHinh < phimTruong.soHoatHinhDau ) {
      nangCapPhimTruong0_nhanVat( &phimTruong, soHoatHinh );
      soHoatHinh++;
   }

   return phimTruong;
}

void chuanBiMayQuayPhimVaMatTroiPhimTruong0( PhimTruong *phimTruong ) {
   
   // ==== máy quay phim
   phimTruong->mayQuayPhim.kieuChieu = kKIEU_CHIEU__PHOI_CANH;
   // ---- vị trí bắt đầu cho máy quay phim
   phimTruong->mayQuayPhim.viTri.x = 0.0f;
   phimTruong->mayQuayPhim.viTri.y = 3.0f;
   phimTruong->mayQuayPhim.viTri.z = 500.0f;
   phimTruong->mayQuayPhim.cachManChieu = 5.0f;
   Vecto trucQuayMayQuayPhim;
   trucQuayMayQuayPhim.x = 0.0f;
   trucQuayMayQuayPhim.y = 1.0f;
   trucQuayMayQuayPhim.z = 0.0f;
   phimTruong->mayQuayPhim.quaternion = datQuaternionTuVectoVaGocQuay( &trucQuayMayQuayPhim, 3.1415926f );
   quaternionQuaMaTran( &(phimTruong->mayQuayPhim.quaternion), phimTruong->mayQuayPhim.xoay );
   //   mayQuayPhim.diChuyen = ();
   
   // ---- mặt trời
   Vecto anhSangMatTroi;
   anhSangMatTroi.x = -1.0f;
   anhSangMatTroi.y = -1.0f;
   anhSangMatTroi.z = -0.5f;
   donViHoa( &anhSangMatTroi );
   phimTruong->matTroi.huongAnh = anhSangMatTroi;
   phimTruong->matTroi.mauAnh.d = 1.0f;
   phimTruong->matTroi.mauAnh.l = 1.0f;
   phimTruong->matTroi.mauAnh.x = 1.0f;
   phimTruong->matTroi.mauAnh.dd = 1.0f;
}

unsigned short datMatBien( VatThe *danhSachVat ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   // ---- mặt biển
   Mau mauBien;
   mauBien.d = 0.5f;
   mauBien.l = 0.5f;
   mauBien.x = 0.5f;
   mauBien.dd = 1.0f;
   mauBien.p = 0.8f;
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   danhSachVat[0].hinhDang.matSong = datMatSong( 3000.0f, 3000.0f, 0.3f, 0.3f, 0.1f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__MAT_SONG;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauBien );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;

   viTri.z = -3000.0f;
   danhSachVat[1].hinhDang.matSong = datMatSong( 3000.0f, 3000.0f, 0.3f, 0.3f, 0.1f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__MAT_SONG;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauBien );
   danhSachVat[1].soHoaTiet = kHOA_TIET__KHONG;

   return 2;
}

unsigned short datCanhGocBoBien( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ==== bờ đất
   Mau mauDat0;
   mauDat0.d = 0.2f;   mauDat0.l = 1.0f;   mauDat0.x = 0.2f;   mauDat0.dd = 1.0f;   mauDat0.p = 0.0f;
   Mau mauDat1;
   mauDat1.d = 0.5f;   mauDat1.l = 1.0f;   mauDat1.x = 0.5f;   mauDat1.dd = 1.0f;   mauDat1.p = 0.0f;

   unsigned short soLuongVat = 0;

   // ---- mặt đất chánh gần - mặt trên cao 10,0
   viTri.x = -100.0f;      viTri.y = 5.0f;        viTri.z = -80.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 239.999f, 9.999f, 199.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 5.0f, 5.0f, 5.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   // ---- mặt đất cao
   mauDat0.d = 0.3f;   mauDat0.l = 1.0f;   mauDat0.x = 0.2f;
   mauDat1.d = 0.7f;   mauDat1.l = 1.0f;   mauDat1.x = 0.5f;
   viTri.x = -157.0f;   viTri.y = 7.5f;      viTri.z = -260.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 119.999f, 14.999f, 99.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 10.0f, 10.0f, 10.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   // ---- mặt đất bên +z
   mauDat0.d = 0.2f;   mauDat0.l = 1.0f;   mauDat0.x = 0.2f;
   mauDat1.d = 0.5f;   mauDat1.l = 1.0f;   mauDat1.x = 0.5f;
   viTri.x = -22.0f;      viTri.y = 2.0f;        viTri.z = 12.0f;

   danhSachVat[soLuongVat].hinhDang.hop = datHop( 100.0f, 6.0f, 40.0f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 2.999f, 2.999f, 2.999f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   // ---- 3 mặt đất thấp bên +x (bờ biển)
   viTri.x = 26.0f;      viTri.y = 4.0f;        viTri.z = -140.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 16.0f, 8.0f, 300.0f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 3.0f, 3.0f, 3.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   viTri.x = 26.0f;      viTri.y = 3.0f;        viTri.z = -490.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 15.0f, 6.0f, 400.0f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 3.0f, 3.0f, 3.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   viTri.x = 26.0f;      viTri.y = 3.5f;        viTri.z = -790.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 17.0f, 7.0f, 200.0f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 3.0f, 3.0f, 3.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
      
   // ---- mặt đất cao xa
   mauDat0.d = 1.0f;   mauDat0.l = 0.5f;   mauDat0.x = 0.2f;
   mauDat1.d = 1.0f;   mauDat1.l = 0.9f;   mauDat1.x = 0.5f;
   viTri.x = -170.0f;   viTri.y = 15.0f;     viTri.z = -820.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 200.0f, 10.0f, 800.0f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   // ---- đất hướng xa -z
   mauDat0.d = 0.3f;   mauDat0.l = 1.0f;   mauDat0.x = 0.2f;
   mauDat1.d = 0.7f;   mauDat1.l = 1.0f;   mauDat1.x = 0.5f;
   viTri.x = -100.0f;   viTri.y = 5.0f;      viTri.z = -330.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 239.999f, 9.999f, 299.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   // ---- đòi bên lề đường
   mauDat0.d = 1.0f;   mauDat0.l = 0.5f;   mauDat0.x = 0.2f;
   mauDat1.d = 1.0f;   mauDat1.l = 0.9f;   mauDat1.x = 0.5f;
   viTri.x = -35.0f;   viTri.y = 12.5f;      viTri.z = -300.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 29.999f, 4.999f, 29.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   viTri.x = -40.0f;   viTri.y = 15.0f;      viTri.z = -290.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 14.999f, 9.999f, 14.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   viTri.x = -110.0f;   viTri.y = 14.0f;      viTri.z = -330.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 44.999f, 7.999f, 54.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   // ---- đất hướng xa (màu cam) -z
   mauDat0.d = 1.0f;   mauDat0.l = 0.5f;   mauDat0.x = 0.2f;
   mauDat1.d = 1.0f;   mauDat1.l = 0.9f;   mauDat1.x = 0.5f;
   viTri.x = -100.0f;   viTri.y = 5.0f;      viTri.z = -730.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 239.999f, 9.999f, 499.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   // ---- hai đòi trên đất cao xa
   viTri.x = -170.0f;   viTri.y = 25.0f;      viTri.z = -630.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 139.999f, 9.999f, 199.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   viTri.x = -50.0f;   viTri.y = 12.0f;      viTri.z = -750.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 49.999f, 3.999f, 399.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   viTri.x = -20.0f;   viTri.y = 11.0f;      viTri.z = -730.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 14.999f, 2.999f, 199.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   viTri.x = -170.0f;   viTri.y = 45.0f;      viTri.z = -530.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 39.999f, 69.999f, 79.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   viTri.x = -170.0f;   viTri.y = 45.0f;      viTri.z = -1030.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 139.999f, 49.999f, 179.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

//   mauDat1.d = 1.0f;   mauDat1.l = 0.9f;   mauDat1.x = 0.5f;
   viTri.x = -165.0f;   viTri.y = 5.0f;      viTri.z = -1230.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 399.999f, 9.999f, 499.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 20.0f, 20.0f, 20.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   // ---- mặt đất chân trời
   viTri.x = -165.0f;   viTri.y = 5.0f;      viTri.z = -1780.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 399.999f, 9.999f, 1099.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 40.0f, 40.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   // ---- núi chân trời -x
   viTri.x = -215.0f;   viTri.y = 25.0f;      viTri.z = -1780.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 399.999f, 39.999f, 499.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 40.0f, 40.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   // ---- hai đòi chân trời +x
   viTri.x = 35.0f;   viTri.y = 15.0f;      viTri.z = -1300.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 29.999f, 29.999f, 59.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 40.0f, 40.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   viTri.x = 45.0f;   viTri.y = 25.0f;      viTri.z = -1315.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 29.999f, 49.999f, 29.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 40.0f, 40.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   viTri.x = 65.0f;   viTri.y = 10.0f;      viTri.z = -1305.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 79.999f, 19.999f, 79.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 40.0f, 40.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   viTri.x = 35.0f;   viTri.y = 12.5f;      viTri.z = -2000.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 29.999f, 24.999f, 59.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 40.0f, 40.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   // ---- mặt đất bên -x, mặt trên cao 10,0
   mauDat0.d = 0.3f;   mauDat0.l = 1.0f;   mauDat0.x = 0.2f;
   mauDat1.d = 0.7f;   mauDat1.l = 1.0f;   mauDat1.x = 0.5f;
   viTri.x = -340.0f;      viTri.y = 5.0f;        viTri.z = -80.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 239.999f, 9.999f, 299.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 8.0f, 8.0f, 8.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   // ----  -x
   mauDat0.d = 0.2f;   mauDat0.l = 1.0f;   mauDat0.x = 0.2f;   mauDat0.dd = 1.0f;   mauDat0.p = 0.0f;
   mauDat1.d = 0.5f;   mauDat1.l = 1.0f;   mauDat1.x = 0.5f;   mauDat1.dd = 1.0f;   mauDat1.p = 0.0f;
   viTri.x = -290.0f;      viTri.y = 15.0f;        viTri.z = 40.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 49.999f, 9.999f, 29.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 8.0f, 8.0f, 8.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   viTri.x = -255.0f;      viTri.y = 12.0f;        viTri.z = 45.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 29.999f, 3.999f, 29.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 8.0f, 8.0f, 8.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   // ---- đảo nhỏ
   viTri.x = -170.0f;      viTri.y = 7.0f;        viTri.z = 48.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 19.999f, 16.999f, 19.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 8.0f, 8.0f, 8.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   viTri.x = -175.0f;      viTri.y = 10.0f;        viTri.z = 53.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 19.999f, 21.999f, 19.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 8.0f, 8.0f, 8.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   // ---- đảo nhỏ
   viTri.x = -20.0f;      viTri.y = 1.5f;        viTri.z = 48.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 29.999f, 4.999f, 19.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 3.0f, 3.0f, 3.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   // ---- cao nguyên thấp bên -x, mặt trên cao 14,0
   mauDat0.d = 0.6f;   mauDat0.l = 1.0f;   mauDat0.x = 0.25f;
   mauDat1.d = 0.8f;   mauDat1.l = 1.0f;   mauDat1.x = 0.55f;
   viTri.x = -360.0f;      viTri.y = 14.0f;        viTri.z = -160.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 299.999f, 7.999f, 299.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 10.0f, 10.0f, 8.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;
   
   viTri.x = -360.0f;      viTri.y = 22.0f;        viTri.z = -100.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 99.999f, 7.999f, 99.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 10.0f, 10.0f, 8.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   viTri.x = -300.0f;      viTri.y = 22.0f;        viTri.z = -160.0f;
   danhSachVat[soLuongVat].hinhDang.hop = datHop( 59.999f, 19.999f, 69.999f, &(danhSachVat[soLuongVat].baoBiVT) );
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 10.0f, 10.0f, 8.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVat++;

   return soLuongVat;
}

unsigned short datCotNghiengSauLongGai( VatThe *danhSachVat ) {
   
   Vecto vectoQuay;
   vectoQuay.x = 0.707f;  vectoQuay.y = 0.707f;   vectoQuay.z = 0.1f;
   Quaternion quaternion = datQuaternionTuVectoVaGocQuay( &vectoQuay, -45.0f );

   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ---- màu cột
   Mau mauDat0;
   mauDat0.d = 0.2f;   mauDat0.l = 1.0f;   mauDat0.x = 1.0f;   mauDat0.dd = 1.0f;   mauDat0.p = 0.0f;
   Mau mauDat1;
   mauDat1.d = 0.5f;   mauDat1.l = 1.0f;   mauDat1.x = 1.0f;   mauDat1.dd = 1.0f;   mauDat1.p = 0.0f;
   
   float mangViTri_x[150] = {
      -204.60f, -160.00f, -150.40f, -185.40f, -129.40f, -169.00f, -150.60f, -133.00f, -139.00f, -190.60f,
      -131.40f, -203.60f, -128.40f, -159.40f, -198.80f, -220.40f, -162.20f, -168.60f, -141.60f, -206.00f,
      -141.80f, -126.00f, -166.60f, -131.60f, -133.60f, -136.00f, -169.20f, -215.40f, -212.40f, -199.60f,
      -130.40f, -134.20f, -189.40f, -198.00f, -179.60f, -217.20f, -173.00f, -136.20f, -168.00f, -218.80f,
      -160.80f, -131.00f, -213.00f, -171.80f, -207.80f, -145.40f, -134.40f, -129.00f, -145.60f, -202.80f,
      -200.80f, -145.80f, -176.60f, -188.60f, -130.60f, -149.80f, -176.00f, -164.20f, -171.40f, -126.20f,
      -121.00f, -164.00f, -163.80f, -150.80f, -184.60f, -131.60f, -141.80f, -123.20f, -217.60f, -175.60f,
      -189.00f, -190.80f, -143.00f, -156.60f, -144.20f, -120.60f, -218.60f, -145.40f, -172.20f, -132.40f,
      -148.20f, -217.40f, -155.60f, -125.40f, -188.00f, -147.20f, -206.20f, -192.40f, -176.80f, -135.60f,
      -141.20f, -125.80f, -173.40f, -163.00f, -152.20f, -185.20f, -195.00f, -148.20f, -186.00f, -207.00f,
};
   float mangViTri_y[100] = {
      11.25f, 29.00f, 16.25f, 19.25f, 10.75f, 11.75f, 21.50f, 15.75f, 23.00f, 12.00f,
      17.25f, 24.00f, 25.50f, 15.25f, 42.25f, 46.75f, 18.50f, 19.25f, 33.75f, 12.00f,
       9.25f, 18.75f, 20.75f, 17.50f, 16.25f, 35.25f, 30.25f, 24.75f, 17.25f, 15.00f,
       9.50f, 25.25f, 25.50f, 14.00f, 14.50f, 13.50f, 37.25f, 17.00f, 17.00f, 23.50f,
      14.75f, 20.75f, 12.25f, 15.00f, 20.00f, 15.50f, 37.00f, 14.75f, 18.75f, 26.00f,
      22.50f, 21.75f, 15.25f, 19.50f,  9.75f, 19.00f, 10.00f, 18.00f, 12.75f, 22.25f,
      17.00f, 25.50f, 14.25f, 24.25f,  8.75f, 19.25f, 18.75f, 12.50f, 13.50f, 29.00f,
      14.50f, 26.25f, 14.50f, 25.75f, 16.75f, 18.50f, 29.75f, 17.00f, 27.50f, 16.00f,
      24.50f, 19.25f, 21.75f, 10.50f, 17.50f, 22.50f, 21.25f, 11.25f,  9.25f, 14.00f,
      12.50f, 15.00f, 20.25f, 43.75f, 16.25f, 13.75f, 32.50f, 16.50f, 13.30f, 25.00f,
};

   float mangViTri_z[100] = {
      -63.40f,-122.00f,  -48.00f, -74.60f, -29.00f,-114.00f, -85.60f, -73.80f, -61.80f, -86.20f,
      -46.40f, -56.00f,  -65.80f, -55.20f, -51.00f, -66.40f,-118.60f, -65.60f, -87.40f, -65.00f,
     -122.20f, -70.60f,  -45.80f, -74.20f, -44.40f -106.80f,-116.40f, -47.20f,-119.60f,-101.40f,
      -72.60f, -37.60f, -120.20f, -59.20f, -43.00f, -98.20f, -72.80f, -91.40f,-107.80f, -67.00f,
      -52.40f, -58.60f, -113.00f, -77.80f, -60.60f, -55.80f, -77.20f,-106.20f, -63.00f,-110.40f,
      -66.40f,-107.00f,  -63.40f, -40.00f, -66.40f,-117.60f, -70.00f, -89.00f, -31.40f,-111.60f,
      -30.80f, -54.60f,  -78.60f, -61.40f, -64.20f, -59.40f, -51.40f, -90.40f, -59.00f,-115.60f,
      -57.00f, -57.00f, -120.20f, -62.60f, -59.20f, -60.40f, -42.60f, -92.60f,-118.20f, -85.40f,
      -21.80f, -49.20f, -112.20f, -75.40f,-119.20f, -51.80f, -97.80f,-102.60f, -97.60f,-115.00f,
     -111.60f, -46.80f, -118.80f,-102.20f, -94.40f, -54.60f, -63.40f, -46.40f, -63.20f, -31.80f
};
   
   float mangBeDai[100] = {
      24.999f, 17.999f, 16.999f, 22.999f, 19.999f, 22.999f, 17.999f, 17.999f, 22.999f, 16.999f,
      22.999f, 24.999f, 22.999f, 18.999f, 27.999f, 15.999f, 19.999f, 25.999f, 16.999f, 20.999f,
      18.999f, 20.999f, 29.999f, 14.999f, 22.999f, 28.999f, 26.999f, 21.999f, 15.999f, 21.999f,
      21.999f, 23.999f, 16.999f, 28.999f, 16.999f, 29.999f, 17.999f, 29.999f, 25.999f, 21.999f,
      26.999f, 27.999f, 24.999f, 23.999f, 22.999f, 26.999f, 22.999f, 15.999f, 16.999f, 15.999f,
      27.999f, 17.999f, 18.999f, 15.999f, 14.999f, 28.999f, 27.999f, 16.999f, 25.999f, 15.999f,
      28.999f, 22.999f, 14.999f, 20.999f, 29.999f, 29.999f, 22.999f, 17.999f, 16.999f, 25.999f,
      14.999f, 15.999f, 17.999f, 17.999f, 15.999f, 14.999f, 14.999f, 26.999f, 18.999f, 22.999f,
      17.999f, 22.999f, 28.999f, 22.999f, 17.999f, 18.999f, 22.999f, 22.999f, 19.999f, 23.999f,
      15.999f, 25.999f, 14.999f, 28.999f, 16.999f, 26.999f, 15.999f, 22.999f, 18.999f, 24.999f,
};

   float mangBeRong[100] = {6.999f, 9.999f, 7.999f, 8.999f, 4.999f, 11.999f, 6.999f, 9.999f, 9.999f, 9.999f,
      8.999f, 11.999f, 5.999f, 9.999f, 5.999f, 9.999f, 9.999f, 6.999f, 4.999f, 4.999f,
      5.999f, 4.999f, 7.999f, 8.999f, 7.999f, 6.999f, 11.999f, 9.999f, 4.999f, 10.999f,
      10.999f, 10.999f, 10.999f, 10.999f, 8.999f, 7.999f, 11.999f, 8.999f, 5.999f, 4.999f,
      8.999f, 9.999f, 10.999f, 5.999f, 11.999f, 9.999f, 7.999f, 5.999f, 7.999f, 7.999f,
      7.999f, 10.999f, 5.999f, 5.999f, 8.999f, 10.999f, 4.999f, 7.999f, 11.999f, 4.999f,
      4.999f, 6.999f, 6.999f, 10.999f, 4.999f, 8.999f, 10.999f, 7.999f, 7.999f, 5.999f,
      6.999f, 11.999f, 7.999f, 11.999f, 6.999f, 8.999f, 10.999f, 4.999f, 10.999f, 10.999f,
      10.999f, 7.999f, 6.999f, 8.999f, 4.999f, 11.999f, 6.999f, 5.999f, 4.999f, 11.999f,
      5.999f, 4.999f, 11.999f, 5.999f, 7.999f, 11.999f, 9.999f, 8.999f, 8.999f, 9.999f
};

   unsigned short soVatThe = 0;
   while( soVatThe < 100 ) {
   // ---- mặt đất chánh gần - mặt trên cao 10,0
      
/*      mangViTri_x[soVatThe] = -(rand() & 0x1ff)*0.2f - 120.0f;
      mangViTri_y[soVatThe] = (rand() & 0x1f)*0.25f + 15.0f;
      mangViTri_z[soVatThe] = -(rand() & 0x1ff)*0.2f - 20.0f;
      mangBeDai[soVatThe] = 14.999f + (rand() & 0xf);
      mangBeRong[soVatThe] = 4.999f + (rand() & 0x7); */
      viTri.x = mangViTri_x[soVatThe];
      viTri.y = mangViTri_y[soVatThe];
      viTri.z = mangViTri_z[soVatThe];
      float beDai = mangBeDai[soVatThe];
      float beRong = mangBeRong[soVatThe];
//      printf( "%d , viTri , %5.3f , %5.3f , %5.3f  beDai , %5.3f  beRong , %5.3f\n", soVatThe, viTri.x, viTri.y, viTri.z, beDai, beRong );
      danhSachVat[soVatThe].hinhDang.hop = datHop( beRong, beDai, beRong, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 5.0f, 5.0f, 5.0f );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__CA_RO;
      soVatThe++;
   }
   /*
   soVatThe = 0;
   while( soVatThe < 100 ) {
      if( soVatThe % 10 == 0 )
         printf( "\n" );
      printf( "%5.2ff, ", mangViTri_x[soVatThe] );
      soVatThe++;
   }
   printf( "\ny\n" );
   
   soVatThe = 0;
   while( soVatThe < 100 ) {
      if( soVatThe % 10 == 0 )
         printf( "\n" );
      printf( "%5.2ff, ", mangViTri_y[soVatThe] );
      soVatThe++;
   }
   printf( "\nz\n" );
   soVatThe = 0;
   while( soVatThe < 100 ) {
      if( soVatThe % 10 == 0 )
         printf( "\n" );
      printf( "%5.2ff, ", mangViTri_z[soVatThe] );
      soVatThe++;
   }
   printf( "\nbeDai\n" );
   soVatThe = 0;
   while( soVatThe < 100 ) {
      if( soVatThe % 10 == 0 )
         printf( "\n" );
      printf( "%5.3ff, ", mangBeDai[soVatThe] );
      soVatThe++;
   }
   printf( "\nbeRong\n" );
   soVatThe = 0;
   while( soVatThe < 100 ) {
      if( soVatThe % 10 == 0 )
         printf( "\n" );
      printf( "%5.3ff, ", mangBeRong[soVatThe] );
      soVatThe++;
   }

   exit(0); */
   return soVatThe;
}

unsigned short datDuongLo( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   Vecto viTri;
   
   Mau mauQuan0;
   Mau mauQuan1;
   Mau mauQuan2;
   
   mauQuan0.d = 0.70f;   mauQuan0.l = 0.70f;   mauQuan0.x = 0.70f;   mauQuan0.dd = 1.0f;   mauQuan0.p = 0.0f;
   mauQuan1.d = 0.75f;   mauQuan1.l = 0.75f;   mauQuan1.x = 0.75f;   mauQuan1.dd = 1.0f;   mauQuan1.p = 0.0f;
   mauQuan2.d = 0.72f;   mauQuan2.l = 0.72f;   mauQuan2.x = 0.72f;   mauQuan2.dd = 1.0f;   mauQuan2.p = 0.0f;

   // ==== đường lộ
   // ---- đầy vòng xuyến
   Mau mauDuongLo;
   mauDuongLo.d = 0.6f;   mauDuongLo.l = 0.6f;   mauDuongLo.x = 0.6f;   mauDuongLo.dd = 1.0f;   mauDuongLo.p = 0.0f;
   viTri.x = 0.0f;      viTri.y = 10.05f;    viTri.z = 0.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[0].hinhDang.hinhTru = datHinhTru( 15.0f, 0.1f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietQuanSongTiaPhai = datHoaTietQuanSongTiaPhai( &mauDuongLo, &mauQuan0, &mauQuan1, &mauQuan2, 0.15f, 0.15f, 0.15f, 0.2f, 0.05f, 0.0f, 16, 10.0f, 15.0f );
   danhSachVat[0].soHoaTiet = kHOA_TIET__QUAN_SONG_TIA_PHAI;
   
   Vecto huongNgang;
   huongNgang.x = 0.0f;   huongNgang.y = 1.0f;   huongNgang.z = 0.0f;
   Vecto huongDoc;
   huongDoc.x = 1.0f;   huongDoc.y = 0.0f;   huongDoc.z = 0.0f;
   // ---- đường hướng -x
   viTri.x = -112.0f;      viTri.y = 10.05f;        viTri.z = 0.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[1].hinhDang.hop = datHop( 200.0f, 0.1f, 10.0f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mauDuongLo, &mauQuan0, &mauQuan1, &mauQuan2, 0.2f, 0.2f, 0.2f, 3.0f, 3.0f, 0.7f, 0.0f, 10.0f );
   danhSachVat[1].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   
   // ---- đường hướng -x
   viTri.x = -212.0f;      viTri.y = 10.05f;        viTri.z = 0.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[2].hinhDang.hop = datHop( 200.0f, 0.1f, 10.0f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[2].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mauDuongLo, &mauQuan0, &mauQuan1, &mauQuan2, 0.2f, 0.2f, 0.2f, 3.0f, 3.0f, 0.7f, 0.0f, 10.0f );
   danhSachVat[2].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;

   // ---- đường hướng -z
   viTri.x = 0.0f;      viTri.y = 10.05f;    viTri.z = -112.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   huongNgang.x = 1.0f;   huongNgang.y = 0.0f;   huongNgang.z = 0.0f;
   huongDoc.x = 0.0f;   huongDoc.y = 0.0f;   huongDoc.z = 1.0f;

   unsigned short soDoan = 0;
   unsigned short chiSo = 3;
   while ( soDoan < 10 ) {
      danhSachVat[chiSo].hinhDang.hop = datHop( 10.0f, 0.1f, 200.0f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mauDuongLo, &mauQuan0, &mauQuan1, &mauQuan2, 0.2f, 0.2f, 0.2f, 3.0f, 3.0f, 0.7f, 0.0f, 10.0f );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
      viTri.z -= 200.0f;
      soDoan++;
      chiSo++;
   }
 
   return soDoan + 3;
}

unsigned short datLanCanDuongLo( VatThe *danhSachVat ) {
   Quaternion quaternion;
   quaternion.w = 1.0f;
   quaternion.x = 0.0f;
   quaternion.y = 0.0f;
   quaternion.z = 0.0f;
   
   Vecto viTri;
   Vecto phongTo;
   
   // ==== bờ đất
   Mau mauNen0;
   Mau mauNen1;
   mauNen0.d = 0.3f;   mauNen0.l = 1.0f;   mauNen0.x = 0.2f;   mauNen0.dd = 1.0f;   mauNen0.p = 0.0f;
   mauNen1.d = 0.5f;   mauNen1.l = 1.0f;   mauNen1.x = 0.5f;   mauNen1.dd = 1.0f;   mauNen1.p = 0.0f;
   
   // ---- cho họa tiết chấm bi
   Mau mauChamBi0;
   Mau mauChamBi1;
   mauChamBi0.d = 0.5f;   mauChamBi0.l = 1.0f;   mauChamBi0.x = 0.5f;   mauChamBi0.dd = 1.0f;   mauChamBi0.p = 0.0f;
   mauChamBi1.d = 1.0f;   mauChamBi1.l = 1.0f;   mauChamBi1.x = 0.4f;   mauChamBi1.dd = 1.0f;   mauChamBi1.p = 0.0f;

   Vecto viTri0;
   Vecto viTri1;
   viTri0.x = 1.0f;   viTri0.y = 0.8f;   viTri0.z = 0.5f;
   viTri1.x = -1.5f;   viTri1.y = 2.5f;   viTri1.z = 0.0f;


   // ---- mặt đất chánh gần - mặt trên cao 5,0 cm
   // ---- đường hướng -x
   viTri.x = -116.0f;      viTri.y = 10.6f;        viTri.z = 0.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   danhSachVat[0].hinhDang.hop = datHop( 200.0f, 1.0f, 0.5f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietHaiChamBi = datHoaTietHaiChamBi( &mauNen0, &mauChamBi0, &mauChamBi1, 0.8f, 3.0f, &viTri0, &viTri1, 10.0f, 5.0f, 5.0f );
   danhSachVat[0].soHoaTiet = kHOA_TIET__HAI_CHAM_BI;
   
   // ---- đường hướng -z
   viTri.x = 0.0f;      viTri.y = 10.6f;    viTri.z = -116.0f;
   phongTo.x = 1.0f;    phongTo.y = 1.0f;    phongTo.z = 1.0f;
   
   // ---- cho họa tiết chấm bi
   mauChamBi0.d = 0.3f;   mauChamBi0.l = 0.3f;   mauChamBi0.x = 1.0f;   mauChamBi0.dd = 1.0f;   mauChamBi0.p = 0.0f;
   mauChamBi1.d = 0.5f;   mauChamBi1.l = 0.5f;   mauChamBi1.x = 1.0f;   mauChamBi1.dd = 1.0f;   mauChamBi1.p = 0.0f;
   
   viTri0.x = 0.0f;   viTri0.y = 2.8f;   viTri0.z = -1.5f;
   viTri1.x = 0.0f;   viTri1.y = 0.3f;   viTri1.z = 2.5f;
   
   unsigned short soDoan = 0;
   unsigned short chiSo = 1;
   while ( soDoan < 10 ) {
      danhSachVat[chiSo].hinhDang.hop = datHop( 0.5f, 1.0f, 200.0f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietHaiChamBi = datHoaTietHaiChamBi( &mauNen0, &mauChamBi0, &mauChamBi1, 3.0f, 0.5f, &viTri0, &viTri1, 5.0f, 5.0f, 10.0f );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__HAI_CHAM_BI;
      viTri.z -= 200.0f;
      soDoan++;
      chiSo++;
   }
   
   return soDoan + 1;
}

unsigned short datThapVongXuyen( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   Vecto viTri;
   
   // ---- đấy
   mau.d = 0.5f;      mau.l = 0.0f;      mau.x = 0.8f;      mau.dd = 1.0f;     mau.p = 0.25f;
   viTri.x = 0.0f;       viTri.y = 10.2f;       viTri.z = 0.0f;
   danhSachVat[0].hinhDang.hinhTru = datHinhTru( 7.5f, 0.2f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   mau.d = 0.1f;      mau.l = 0.0f;      mau.x = 0.35f;      mau.dd = 1.0f;     mau.p = 0.0f;
   Mau mauOc0;
   mauOc0.d = 0.8f;      mauOc0.l = 0.5f;      mauOc0.x = 0.3f;      mauOc0.dd = 1.0f;      mauOc0.p = 0.2f;
   Mau mauOc1;
   mauOc1.d = 0.8f;      mauOc1.l = 0.25f;      mauOc1.x = 0.15f;      mauOc1.dd = 1.0f;      mauOc1.p = 0.2f;
   Mau mauOc2;
   mauOc2.d = 0.3f;      mauOc2.l = 0.0f;      mauOc2.x = 0.3f;      mauOc2.dd = 1.0f;      mauOc2.p = 0.2f;
   viTri.x = 0.0f;       viTri.y = 10.4f;       viTri.z = 0.0f;
   danhSachVat[1].hinhDang.hinhTru = datHinhTru( 7.2f, 0.2f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.05f, 0.08f, 0.15f, 0.0f, 0.165f, 16 );
   danhSachVat[1].soHoaTiet = kHOA_TIET__QUAN_XOAY;
   
   // ---- lớp trên đấy
   viTri.x = 0.0f;       viTri.y = 11.25f;       viTri.z = 0.0f;
   danhSachVat[2].hinhDang.hinhNon = datHinhNon( 7.2f, 4.5f, 1.5f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[2].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.05f, 0.08f, 0.15f, 0.3f, 0.0f, 16 );
   danhSachVat[2].soHoaTiet = kHOA_TIET__QUAN_XOAY;
   
   // ---- lớp trên đấy
   mau.d = 1.0f;      mau.l = 0.4f;      mau.x = 0.0f;      mau.dd = 1.0f;    mau.p = 0.25f;
   viTri.x = 0.0f;       viTri.y = 12.15f;       viTri.z = 0.0f;
   danhSachVat[3].hinhDang.hinhNon = datHinhNon( 4.5f, 5.0f, 0.3f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[3].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternion, &viTri );
   danhSachVat[3].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[3].soHoaTiet = kHOA_TIET__KHONG;

   viTri.x = 0.0f;       viTri.y = 12.45f;       viTri.z = 0.0f;
   danhSachVat[4].hinhDang.hinhTru = datHinhTru( 5.0f, 0.3f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[4].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternion, &viTri );
   danhSachVat[4].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[4].soHoaTiet = kHOA_TIET__KHONG;
   
   // ---- hai vòng trên
   mau.d = 0.92f;      mau.l = 0.72f;      mau.x = 1.0f;      mau.dd = 1.0f;    mau.p = 0.25f;
   viTri.x = 0.0f;       viTri.y = 12.65f;       viTri.z = 0.0f;
   danhSachVat[5].hinhDang.hinhTru = datHinhTru( 4.5f, 0.1f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[5].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[5].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.x = 0.0f;       viTri.y = 12.75f;       viTri.z = 0.0f;
   danhSachVat[6].hinhDang.hinhTru = datHinhTru( 4.2f, 0.1f, &(danhSachVat[6].baoBiVT) );
   danhSachVat[6].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[6].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6]), &phongTo, &quaternion, &viTri );
   danhSachVat[6].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[6].soHoaTiet = kHOA_TIET__KHONG;

   // ---- đấy tháp
   mau.d = 0.3f;      mau.l = 0.0f;      mau.x = 0.15f;      mau.dd = 1.0f;      mau.p = 0.25f;
   mauOc0.d = 0.55f;     mauOc0.l = 0.0f;      mauOc0.x = 0.35f;     mauOc0.dd = 1.0f;   mauOc0.p = 0.25f;
   mauOc1.d = 0.55f;     mauOc1.l = 0.0f;      mauOc1.x = 0.35f;     mauOc1.dd = 1.0f;   mauOc1.p = 0.25f;
   mauOc2.d = 0.35f;     mauOc2.l = 0.0f;      mauOc2.x = 0.18f;     mauOc2.dd = 1.0f;   mauOc2.p = 0.25f;
   viTri.x = 0.0f;       viTri.y = 14.85f;       viTri.z = 0.0f;
   danhSachVat[7].hinhDang.hinhNon = datHinhNon( 2.0f, 0.5f, 4.1f, &(danhSachVat[7].baoBiVT) );
   danhSachVat[7].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[7].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[7]), &phongTo, &quaternion, &viTri );
   danhSachVat[7].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.1f, 0.1f, 0.1f, 3.0f, 0.0f, 2 );
   danhSachVat[7].soHoaTiet = kHOA_TIET__QUAN_XOAY;

   // ---- đỉnh tháp
   mau.d = 1.0f;      mau.l = 0.8f;      mau.x = 0.0f;      mau.dd = 1.0f;     mau.p = 0.25f;
   mauOc0.d = 1.0f;     mauOc0.l = 0.8f;      mauOc0.x = 0.4f;     mauOc0.dd = 1.0f;   mauOc0.p = 0.25f;
   mauOc1.d = 1.0f;     mauOc1.l = 0.8f;      mauOc1.x = 0.4f;     mauOc1.dd = 1.0f;   mauOc1.p = 0.25f;
   mauOc2.d = 1.0f;     mauOc2.l = 0.6f;      mauOc2.x = 0.0f;     mauOc2.dd = 1.0f;   mauOc2.p = 0.25f;
   viTri.x = 0.0f;     viTri.y = 18.15f;    viTri.z = 0.0f;
   danhSachVat[8].hinhDang.hinhNon = datHinhNon( 0.4f, 0.0f, 2.5f, &(danhSachVat[8].baoBiVT) );
   danhSachVat[8].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[8].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[8]), &phongTo, &quaternion, &viTri );
   danhSachVat[8].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.15f, 0.15f, 0.15f, 4.0f, 0.0f, 2 );
   danhSachVat[8].soHoaTiet = kHOA_TIET__QUAN_XOAY;
   
   // ---- các hình nón quanh tháp
   mau.d = 0.1f;      mau.l = 0.0f;      mau.x = 0.33f;      mau.dd = 1.0f;     mau.p = 0.0f;
   mauOc0.d = 0.8f;      mauOc0.l = 0.5f;      mauOc0.x = 0.3f;      mauOc0.dd = 1.0f;   mauOc0.p = 0.0f;
   mauOc1.d = 1.0f;      mauOc1.l = 0.5f;      mauOc1.x = 0.3f;      mauOc1.dd = 1.0f;   mauOc1.p = 0.0f;
   mauOc2.d = 0.3f;      mauOc2.l = 0.0f;      mauOc2.x = 0.3f;      mauOc2.dd = 1.0f;   mauOc2.p = 0.0f;
   float goc = 0.0f;
   unsigned char soHinhNon = 0;
   while ( soHinhNon < 8 ) {
      unsigned char chiSo = 9 + soHinhNon;
      viTri.x = 3.0f*cosf( goc );       viTri.y = 13.55f;       viTri.z = 3.0f*sinf( goc );
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 0.7f, 0.0f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.1f, 0.1f, 0.15f, 2.0f, 0.0f, 2 );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__QUAN_XOAY;
      soHinhNon++;
      goc += 3.14159f/4.0f;
   }
   
   return 17;
}

unsigned short datDayCotQuangVongXuyen( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   mau.d = 0.5f;    mau.l = 0.3f;    mau.x = 0.6f;    mau.dd = 1.0f;    mau.p = 0.5f;
   Vecto viTri;
   
   float goc = 3.1415926f*0.8f;

   unsigned short soVatThe = 0;
   unsigned char soCot = 0;
   while ( soCot < 12 ) {
      viTri.x = 17.5f*cosf( goc );       viTri.y = 10.2f;       viTri.z = 17.5f*sinf( goc );

      danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 1.2f, 0.4f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
      
      viTri.y += 0.25f;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 1.2f, 1.1f, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
   
      soCot++;
      goc += -3.14159f*0.1f;
   }

   return soVatThe;
}

unsigned short datLongSaoGai( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   Mau mauNoi;
   Mau mauNgoai;
   Mau mauVanh;
   Vecto viTri;
  
   // ---- đấy - lớp 0
   unsigned short soVatThe = 0;
   mau.d = 1.0f;      mau.l = 0.8f;      mau.x = 0.8f;      mau.dd = 1.0f;     mau.p = 0.15f;
   mauNoi.d = 1.0f;   mauNoi.l = 0.3f;   mauNoi.x = 0.35f;   mauNoi.dd = 1.0f;   mauNoi.p = 0.05f;
   mauNgoai.d = 1.0f; mauNgoai.l = 0.7f; mauNgoai.x = 0.7f; mauNgoai.dd = 1.0f; mauNgoai.p = 0.05f;
   mauVanh.d = 1.0f;  mauVanh.l = 0.2f;  mauVanh.x = 0.33f;  mauVanh.dd = 1.0f;  mauVanh.p = 0.01f;
   viTri.x = -52.0f;      viTri.y = 12.15f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 32.0f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietBongVong = datHoaTietBongVong( &mau, &mauNoi, &mauNgoai, &mauVanh, 29.0f, 30.5f, 30.7f, &mauNoi, &mauNgoai, 0.5f, 85 );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__BONG_VONG;
   soVatThe++;

   // ---- đấy - lớp 1
   mau.d = 1.0f;      mau.l = 0.0f;      mau.x = 0.1f;      mau.dd = 1.0f;
   mauNoi.d = 1.0f;   mauNoi.l = 0.2f;   mauNoi.x = 0.25f;   mauNoi.dd = 1.0f;   mauNoi.p = 0.05f;
   mauNgoai.d = 0.7f; mauNgoai.l = 0.0f; mauNgoai.x = 0.05f; mauNgoai.dd = 1.0f; mauNgoai.p = 0.05f;
   mauVanh.d = 1.0f;  mauVanh.l = 0.5f;  mauVanh.x = 0.55f;  mauVanh.dd = 1.0f;  mauVanh.p = 0.01f;
   viTri.x = -52.0f;      viTri.y = 12.45f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 26.0f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietBongVong = datHoaTietBongVong( &mau, &mauNoi, &mauNgoai, &mauVanh, 23.0f, 24.5f, 24.7f, &mauNoi, &mauNgoai, 0.5f, 69 );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__BONG_VONG;
   soVatThe++;

   // ---- đấy - lớp 2
   mau.d = 1.0f;      mau.l = 0.8f;      mau.x = 0.8f;      mau.dd = 1.0f;
   mauNoi.d = 1.0f;   mauNoi.l = 0.3f;   mauNoi.x = 0.35f;   mauNoi.dd = 1.0f;   mauNoi.p = 0.05f;
   mauNgoai.d = 1.0f; mauNgoai.l = 0.7f; mauNgoai.x = 0.7f; mauNgoai.dd = 1.0f; mauNgoai.p = 0.05f;
   mauVanh.d = 1.0f;  mauVanh.l = 0.2f;  mauVanh.x = 0.33f;  mauVanh.dd = 1.0f;  mauVanh.p = 0.01f;
   viTri.x = -52.0f;      viTri.y = 12.75f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 20.0f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietBongVong = datHoaTietBongVong( &mau, &mauNoi, &mauNgoai, &mauVanh, 17.0f, 18.5f, 18.7f, &mauNoi, &mauNgoai, 0.5f, 53 );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__BONG_VONG;
   soVatThe++;
   
   // ---- đấy - lớp 3
   mau.d = 1.0f;      mau.l = 0.0f;      mau.x = 0.1f;      mau.dd = 1.0f;
   mauNoi.d = 1.0f;   mauNoi.l = 0.2f;   mauNoi.x = 0.25f;   mauNoi.dd = 1.0f;   mauNoi.p = 0.05f;
   mauNgoai.d = 0.7f; mauNgoai.l = 0.0f; mauNgoai.x = 0.05f; mauNgoai.dd = 1.0f; mauNgoai.p = 0.05f;
   mauVanh.d = 1.0f;  mauVanh.l = 0.5f;  mauVanh.x = 0.55f;  mauVanh.dd = 1.0f;  mauVanh.p = 0.01f;
   viTri.x = -52.0f;      viTri.y = 13.05f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 15.0f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietBongVong = datHoaTietBongVong( &mau, &mauNoi, &mauNgoai, &mauVanh, 12.0f, 13.5f, 13.7f, &mauNoi, &mauNgoai, 0.5f, 40 );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__BONG_VONG;
   soVatThe++;

   // ---- đấy - lớp 4
   mau.d = 1.0f;      mau.l = 0.8f;      mau.x = 0.8f;      mau.dd = 1.0f;
   mauNoi.d = 1.0f;   mauNoi.l = 0.3f;   mauNoi.x = 0.35f;   mauNoi.dd = 1.0f;   mauNoi.p = 0.05f;
   mauNgoai.d = 1.0f; mauNgoai.l = 0.7f; mauNgoai.x = 0.7f; mauNgoai.dd = 1.0f; mauNgoai.p = 0.05f;
   mauVanh.d = 1.0f;  mauVanh.l = 0.2f;  mauVanh.x = 0.33f;  mauVanh.dd = 1.0f;  mauVanh.p = 0.01f;
   viTri.x = -52.0f;      viTri.y = 13.35f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 11.0f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietBongVong = datHoaTietBongVong( &mau, &mauNoi, &mauNgoai, &mauVanh, 8.0f, 9.5f, 9.7f, &mauNoi, &mauNgoai, 0.5f, 29 );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__BONG_VONG;
   soVatThe++;
   
   // ---- đấy - lớp 5
   mau.d = 0.6f;      mau.l = 0.0f;      mau.x = 0.1f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 13.85f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 6.0f, 1.0f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   // ---- nền cho cột
   mau.d = 0.6f;      mau.l = 0.0f;      mau.x = 0.1f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 14.6f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 5.3f, 0.5f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 15.25f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 5.3f, 4.5f, 0.8f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[7]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   // ---- cột chánh
   mau.d = 1.0f;      mau.l = 0.7f;      mau.x = 0.5f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 18.1f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 4.0f, 4.9f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   viTri.x = -52.0f;      viTri.y = 20.7f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 4.0f, 3.7f, 0.3, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   // ----
   mau.d = 0.0f;      mau.l = 0.35f;      mau.x = 1.0f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 20.9f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.4f, 3.5f, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 21.15f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 3.5f, 0.4f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 21.4f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.5f, 3.4f, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   // ----
   mau.d = 0.933f;    mau.l = 0.533f;    mau.x = 0.367f;    mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 21.6f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.7f, 4.0f, 0.3, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 21.9f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 4.0f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 22.2f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 4.0f, 3.7f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
 
   // ----
   mau.d = 0.1f;      mau.l = 0.40f;      mau.x = 1.0f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 22.4f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.4f, 3.5f, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 22.65f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 3.5f, 0.4f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 22.9f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.5f, 3.4f, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   // ----
   mau.d = 0.867f;    mau.l = 0.367f;    mau.x = 0.233f;    mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 23.1f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.7f, 4.0f, 0.3, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   viTri.x = -52.0f;      viTri.y = 23.4f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 4.0f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 23.7f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 4.0f, 3.7f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   // ---- 
   mau.d = 0.2f;      mau.l = 0.45f;      mau.x = 1.0f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 23.9f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.4f, 3.5f, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   viTri.x = -52.0f;      viTri.y = 24.15f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 3.5f, 0.4f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   viTri.x = -52.0f;      viTri.y = 24.35f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.5f, 3.4, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   // ---- trên cột chánh
   mau.d = 0.8f;      mau.l = 0.2f;      mau.x = 0.1f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 24.55f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.7f, 4.0f, 0.3, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 24.95f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 4.0f, 0.5f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   viTri.x = -52.0f;      viTri.y = 25.325f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 4.0f, 3.75f, 0.25f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 25.575f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.7f, 3.45f, 0.25f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   // ---- sàn lồng - vật thể bool
   mau.d = 1.0f;      mau.l = 0.1f;      mau.x = 0.05f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 25.75f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].loai = kLOAI_VAT_THE__BOOL;
   danhSachVat[soVatThe].mucDichBool = 1;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   
   danhSachVat[soVatThe].soLuongVatThe = 2;
   danhSachVat[soVatThe].danhSachVatThe = malloc( 2*sizeof(VatThe) );
   
   // vị trí tương đối
   viTri.x = 0.0f;      viTri.y = 0.0f;        viTri.z = 0.0f;
   danhSachVat[soVatThe].danhSachVatThe[0].hinhDang.hinhTru = datHinhTru( 3.2f, 0.1f, &(danhSachVat[soVatThe].danhSachVatThe[0].baoBiVT) );
   danhSachVat[soVatThe].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].danhSachVatThe[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe].danhSachVatThe[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].danhSachVatThe[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].danhSachVatThe[0].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[soVatThe].danhSachVatThe[0].giaTri = 1;
   
   viTri.x = 0.0f;      viTri.y = 0.05f;        viTri.z = 0.0f;
   danhSachVat[soVatThe].danhSachVatThe[1].hinhDang.hinhTru = datHinhTru( 1.5f, 0.1f, &(danhSachVat[soVatThe].danhSachVatThe[1].baoBiVT) );
   danhSachVat[soVatThe].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].danhSachVatThe[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe].danhSachVatThe[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].danhSachVatThe[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].danhSachVatThe[1].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[soVatThe].danhSachVatThe[1].giaTri = -1;
   // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
   tinhBaoBiVTChoVatTheGhep( &(danhSachVat[soVatThe]) );
   soVatThe++;

   // ---- nắp lồng
   mau.d = 0.0f;      mau.l = 0.0f;      mau.x = 0.85f;
   viTri.x = -52.0f;      viTri.y = 29.65f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.2f, 3.5f, 0.2f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

    mau.d = 0.9f;      mau.l = 0.2f;      mau.x = 0.1f;      mau.dd = 1.0f;
   viTri.x = -52.0f;      viTri.y = 30.0f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 4.0f, 0.5f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   viTri.x = -52.0f;      viTri.y = 30.5f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 4.0f, 3.5f, 0.5f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;
   
   // ---- đỉnh nắp
   mau.d = 1.0f;      mau.l = 0.2f;      mau.x = 0.0f;    mau.p = 0.4f;
   viTri.x = -52.0f;      viTri.y = 31.0f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 1.5f, 1.0f, 0.5f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   viTri.x = -52.0f;      viTri.y = 31.35f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 0.8f, 0.2f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   viTri.x = -52.0f;      viTri.y = 31.85f;        viTri.z = -52.0f;
   danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 0.8f, 0.0f, 0.8f, &(danhSachVat[soVatThe].baoBiVT) );
   danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soVatThe++;

   return soVatThe;
}

unsigned short datVuonQuangDatLongSaoGai( VatThe *danhSachVat ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   Vecto viTri;
   
   // ---- màu cho ca rô
   Mau mau0;
   Mau mau1;

   // ---- nền cho lầng sao gai
   mau0.d = 1.0f;   mau0.l = 0.1f;   mau0.x = 0.2f;   mau0.dd = 1.0f;   mau0.p = 0.0f;
   mau1.d = 1.0f;   mau1.l = 0.15f;   mau1.x = 0.25f;   mau1.dd = 1.0f;   mau1.p = 0.0f;
   viTri.x = -52.0f;      viTri.y = 11.0f;        viTri.z = -52.0f;
   danhSachVat[0].hinhDang.hop = datHop( 74.999f, 1.999f, 74.999f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 2.0f, 2.0f, 2.0f );
   danhSachVat[0].soHoaTiet = kHOA_TIET__CA_RO;

   // ---- lối ra phía trước
   viTri.x = -11.0f;      viTri.y = 11.0f;        viTri.z = -52.0f;
   danhSachVat[1].hinhDang.hop = datHop( 9.999f, 1.999f, 9.999f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 2.0f, 2.0f, 2.0f );
   danhSachVat[1].soHoaTiet = kHOA_TIET__CA_RO;
   
   // ---- lối ra phía sau
   viTri.x = -93.0f;      viTri.y = 11.0f;        viTri.z = -52.0f;
   danhSachVat[2].hinhDang.hop = datHop( 9.999f, 1.999f, 9.999f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[2].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 2.0f, 2.0f, 2.0f );
   danhSachVat[2].soHoaTiet = kHOA_TIET__CA_RO;

   // ====
   // ---- hình nón sáng
   Mau mauNen0;
   Mau mauOc00;
   Mau mauOc01;
   Mau mauOc02;

   mauNen0.d = 1.0f;   mauNen0.l = 0.8f;   mauNen0.x = 0.8f;  mauNen0.dd = 1.0f;    mauNen0.p = 0.0f;
   mauOc00.d = 1.0f;    mauOc00.l = 0.4f;    mauOc00.x = 0.4f;   mauOc00.dd = 1.0f;     mauOc00.p = 0.0f;
   mauOc01.d = 1.0f;    mauOc01.l = 0.6f;    mauOc01.x = 0.6f;   mauOc01.dd = 1.0f;     mauOc01.p = 0.0f;
   mauOc02.d = 1.0f;    mauOc02.l = 0.7f;    mauOc02.x = 0.7f;   mauOc02.dd = 1.0f;     mauOc02.p = 0.0f;

   // ---- hình nón tối
   Mau mauNen1;
   Mau mauOc10;
   Mau mauOc11;
   Mau mauOc12;
   mauNen1.d = 0.5f;     mauNen1.l = 0.15f;    mauNen1.x = 0.25f;   mauNen1.dd = 1.0f;   mauNen1.p = 0.0f;
   mauOc10.d = 0.45f;    mauOc10.l = 0.13f;    mauOc10.x = 0.21f;   mauOc10.dd = 1.0f;   mauOc10.p = 0.0f;
   mauOc11.d = 0.41f;    mauOc11.l = 0.115f;   mauOc11.x = 0.15f;   mauOc11.dd = 1.0f;   mauOc11.p = 0.0f;
   mauOc12.d = 0.37f;    mauOc12.l = 0.1f;     mauOc12.x = 0.1f;    mauOc12.dd = 1.0f;   mauOc12.p = 0.0f;

   // ---- cạnh +x
   viTri.y = 10.5f;
   viTri.z = -52.0f + 32.0f;
   unsigned char chiSo = 3;
   while( chiSo < 20 ) {  // 17 cái
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 5.0f, 1.5f, 2.0f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;

      if( chiSo & 0x01 ) {
         danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen0, &mauOc00, &mauOc01, &mauOc02, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 1 );
         viTri.x = -52.0f + 42.5f;
      }
      else {
         danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen1, &mauOc10, &mauOc11, &mauOc12, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 1 );
         viTri.x = -52.0f + 40.5f;
      }
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__QUAN_XOAY;
      viTri.z -= 4.0f;
      chiSo++;
   }

   // ---- cạnh -x

   viTri.z = -52.0f + 32.0f;
   while( chiSo < 37 ) {  // 15 cái
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 5.0f, 1.5f, 2.0f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      if( chiSo & 0x01 ) {
         danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen0, &mauOc00, &mauOc01, &mauOc02, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 1 );
         viTri.x = -52.0f - 40.5f;
      }
      else {
         danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen1, &mauOc10, &mauOc11, &mauOc12, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 1 );
         viTri.x = -52.0f - 42.5f;
      }
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__QUAN_XOAY;
      
      viTri.z -= 4.0f;
      chiSo++;
   }
   
   // ---- cạnh +z
   viTri.x = -52.0f + 32.0f;
   viTri.y = 10.5f;

   while( chiSo < 54 ) {  // 15 cái
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 5.0f, 1.5f, 2.0f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      if( chiSo & 0x01 ) {
         danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen0, &mauOc00, &mauOc01, &mauOc02, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 1 );
         viTri.z = -52.0f + 42.5f;
      }
      else {
         danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen1, &mauOc10, &mauOc11, &mauOc12, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 1 );
         viTri.z = -52.0f + 40.5f;
      }
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__QUAN_XOAY;
      viTri.x -= 4.0f;
      chiSo++;
   }
   
   // ---- cạnh -z
   viTri.x = -52.0f + 32.0f;
   viTri.y = 10.5f;

   while( chiSo < 71 ) {  // 15 cái
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 5.0f, 1.5f, 2.0f , &(danhSachVat[chiSo].baoBiVT));
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      if( chiSo & 0x01 ) {
         danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen0, &mauOc00, &mauOc01, &mauOc02, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 1 );
         viTri.z = -52.0f - 40.5f;
      }
      else {
         danhSachVat[chiSo].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen1, &mauOc10, &mauOc11, &mauOc12, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 1 );
         viTri.z = -52.0f - 42.5f;
      }
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__QUAN_XOAY;
      viTri.x -= 4.0f;
      chiSo++;
   }
   return 71;
}


unsigned short datLanCanNgoaiQuanhLongSaoGai( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   Vecto viTri;

   // ---- đấy đỏ
   mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.2f;    mau.dd = 1.0f;

   float buocGoc = 3.1415926f/12.0f;
   float goc = buocGoc;
   unsigned char chiSo = 0;
   while( chiSo < 66 ) {  // 11 cái * 6 bộ phần
      mau.d = 0.7f;   mau.l = 0.0f;   mau.x = 0.1f;    mau.dd = 1.0f;    mau.p = 0.0f;
      viTri.x = 29.0f*cosf( goc ) - 52.0f;     viTri.z = 29.0f*sinf( goc ) - 52.0f;
      viTri.y = 12.6f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.3f, 0.6f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;

      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.2f;
      viTri.y += 1.05f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.0f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;

      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;   mau.p = 0.1f;
      viTri.y += 0.85f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 1.7f, 0.2f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      viTri.y += 0.45f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 1.7f, 1.0f, 0.7f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.0f;    mau.dd = 0.8f;
      viTri.y += 0.5f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 1.0f, 0.7f, 0.3f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
   
      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;    mau.dd = 1.0f;
      viTri.y += 0.5f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 0.7f, 0.0f, 0.7f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      goc += buocGoc;
   }
   
   goc = 3.1415926f + buocGoc;
   while( chiSo < 132 ) {  // 11 cái * 6 bộ phần
      mau.d = 0.7f;   mau.l = 0.0f;   mau.x = 0.1f;    mau.dd = 1.0f;    mau.p = 0.0f;
      viTri.x = 29.0f*cosf( goc ) - 52.0f;       viTri.z = 29.0f*sinf( goc ) - 52.0f;
      viTri.y = 12.6f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.3f, 0.6f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.2f;
      viTri.y += 1.05f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.0f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;    mau.p = 0.1f;
      viTri.y += 0.85f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 1.7f, 0.2f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;

      viTri.y += 0.45f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 1.7f, 1.0f, 0.7f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;

      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.0f;    mau.dd = 0.8f;
      viTri.y += 0.5f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 1.0f, 0.7f, 0.3f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;    mau.dd = 1.0f;
      viTri.y += 0.5f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 0.7f, 0.0f, 0.7f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      goc += buocGoc;
   }

   // ---- đấy trắng
   mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;    mau.dd = 1.0f;   mau.p = 0.0f;
   goc = buocGoc*1.5f;

   while( chiSo < 152 ) {  // 10 cái * 2 bộ phần
      viTri.x = 29.0f*cosf( goc ) - 52.0f;      viTri.z = 29.0f*sinf( goc ) - 52.0f;
      
      viTri.y = 12.59f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.3f, 0.58f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;

      viTri.y = 13.8f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.2f, 0.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      goc += buocGoc;
      chiSo++;
   }
   
   goc = 3.1415926f + buocGoc*1.5f;
   while( chiSo < 172 ) {  // 10 cái * 2 bộ phần
      viTri.x = 29.0f*cosf( goc ) - 52.0f;    viTri.z = 29.0f*sinf( goc ) - 52.0f;

      viTri.y = 12.59f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.3f, 0.58f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;

      viTri.y = 13.8f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.2f, 0.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      goc += buocGoc;
      chiSo++;
   }

   return 172;
}


unsigned short datBonThapVuonSaoGai( VatThe *danhSachVat ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   Vecto viTri;
   
   Mau mauOc0;
   Mau mauOc1;
   Mau mauOc2;

   mauOc0.d = 0.5f;     mauOc0.l = 0.0f;     mauOc0.x = 0.0f;    mauOc0.dd = 1.0f;    mauOc0.p = 0.05f;
   mauOc1.d = 1.0f;     mauOc1.l = 0.25f;     mauOc1.x = 0.35f;    mauOc1.dd = 1.0f;    mauOc1.p = 0.05f;
   mauOc2.d = 1.0f;     mauOc2.l = 0.55f;     mauOc2.x = 0.65f;    mauOc2.dd = 1.0f;    mauOc2.p = 0.05f;
   
   // ---- 
   float viTriX[4] = {-21.0f, -21.0f, -83.0f, -83.0f};
   float viTriZ[4] = {-21.0f, -83.0f, -83.0f, -21.0f};
   
   unsigned short soVatThe = 0;
   unsigned char soThap = 0;

     // ---- THÁP SỐ 0
   while ( soThap < 4 ) {

      // ---- nền tháp
      mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;    mau.dd = 1.0f;    mau.p = 0.1f;
      viTri.x = viTriX[soThap];      viTri.y = 12.125f;        viTri.z = viTriZ[soThap];
      danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 5.0f, 0.25f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
 
      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;   mau.p = 0.5f;
      viTri.y += 0.25f;
      danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 4.7f, 0.25f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
      // ---- ba hình nón chất lên
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.1f;    mau.p = 0.1f;
      viTri.y += 0.375;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 4.0f, 3.8f, 0.5f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;

      viTri.y += 0.5;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.6f, 3.4f, 0.5f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
      
      viTri.y += 0.5;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 3.2f, 3.0f, 0.5f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;

      mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;
      viTri.y += 0.4f;
      danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 2.3f, 0.3f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
      // ---- đỉnh tháp
      mau.d = 0.8f;   mau.l = 0.0f;   mau.x = 0.0f;
      viTri.y += 2.15f;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 1.7f, 0.0f, 4.0f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 2 );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__QUAN_XOAY;
      soVatThe++;

      soThap++;
   }

   // ----                góc (-21.0; -21.0f)    (21.0f, -83.0f)  (-83.0f; -83.0f)  (-21.0f, -83.0f)
   float viTriX_thapNho[8] = {-30.0f, -18.0f,   -18.0f, -30.0f,   -74.0, -86.0f,   -86.0f, -74.0f};
   float viTriZ_thapNho[8] = {-18.0f, -30.0f,   -74.0f, -86.0f,   -86.0, -74.0f,   -30.0f, -18.0f};
   
   soThap = 0;
   while ( soThap < 8 ) {
      // ---- nền tháp
      mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;    mau.dd = 1.0f;    mau.p = 0.1f;
      viTri.x = viTriX_thapNho[soThap];      viTri.y = 12.125f;        viTri.z = viTriZ_thapNho[soThap];
      danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 2.0f, 0.25f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
      
      mau.d = 1.0f;   mau.l = 0.8f;   mau.x = 0.8f;   mau.p = 0.3f;
      viTri.y += 0.25f;
      danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 1.7f, 0.25f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
      
      // ---- hình nón chất lên
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.1f;    mau.p = 0.1f;
      viTri.y += 0.625;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 1.4f, 1.0f, 1.0f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;

      // ---- đỉnh tháp
      mau.d = 0.8f;   mau.l = 0.0f;   mau.x = 0.0f;
      viTri.y += 1.5f;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 0.8f, 0.0f, 2.0f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.1f, 0.1f, 0.1f, 2.0f, 0.0f, 2 );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__QUAN_XOAY;
      soVatThe++;

      soThap++;
   }
   
   // ----                     góc (-21.0; -21.0f)    (21.0f, -83.0f)  (-83.0f; -83.0f)  (-21.0f, -83.0f)
   float viTriX_thapNhoNhat[8] = {-39.0f, -17.0f,   -17.0f, -39.0f,   -65.0, -87.0f,    -87.0f, -65.0f};
   float viTriZ_thapNhoNhat[8] = {-17.0f, -39.0f,   -65.0f, -87.0f,   -87.0f, -65.0f,   -39.0f, -17.0f};

   soThap = 0;
   while ( soThap < 8 ) {
      // ---- nền tháp
      mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;    mau.dd = 1.0f;    mau.p = 0.1f;
      viTri.x = viTriX_thapNhoNhat[soThap];      viTri.y = 12.125f;        viTri.z = viTriZ_thapNhoNhat[soThap];
      danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 1.0f, 0.25f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
      
      // ---- hình nón chất lên
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.1f;    mau.p = 0.1f;
      viTri.y += 0.625;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 0.7f, 0.5f, 1.0f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      soVatThe++;
      
      // ---- đỉnh tháp
      mau.d = 0.8f;   mau.l = 0.0f;   mau.x = 0.0f;
      viTri.y += 1.5f;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 0.4f, 0.0f, 2.0f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.2f, 0.2f, 0.12f, 2.0f, 0.0f, 2 );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__QUAN_XOAY;
      soVatThe++;
      
      soThap++;
   }

   
   return soVatThe;
}

unsigned short datLanCanTrongQuanhLongSaoGai( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   Vecto viTri;
   
   // ---- đấy đỏ
   mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.2f;    mau.dd = 1.0f;   mau.p = 0.0f;
   
   float buocGoc = 3.1415926f/8.0f;
   float goc = buocGoc;
   unsigned char chiSo = 0;
   while( chiSo < 42 ) {  // 7 cái * 6 bộ phần
      mau.d = 0.7f;   mau.l = 0.0f;   mau.x = 0.1f;    mau.dd = 1.0f;     mau.p = 0.0f;
      viTri.x = 17.5f*cosf( goc ) - 52.0f;     viTri.z = 17.5f*sinf( goc ) - 52.0f;
      viTri.y = 13.2f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.3f, 0.6f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.2f;
      viTri.y += 1.05f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.0f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;   mau.p = 0.1f;
      viTri.y += 0.85f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 1.7f, 0.2f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      viTri.y += 0.45f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 1.7f, 1.0f, 0.7f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.0f;    mau.dd = 0.5f;
      viTri.y += 0.5f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 1.0f, 0.7f, 0.3f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;    mau.dd = 1.0f;
      viTri.y += 0.5f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 0.7f, 0.0f, 0.7f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      goc += buocGoc;
   }

   goc = 3.1415926f + buocGoc;
   while( chiSo < 84 ) {  // 7 cái * 6 bộ phần
      mau.d = 0.7f;   mau.l = 0.0f;   mau.x = 0.1f;    mau.dd = 1.0f;    mau.p = 0.0f;
      viTri.x = 17.5f*cosf( goc ) - 52.0f;       viTri.z = 17.5f*sinf( goc ) - 52.0f;
      viTri.y = 13.2f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.3f, 0.6f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.2f;    mau.dd = 1.0f;
      viTri.y += 1.05f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.0f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;    mau.dd = 1.0f;    mau.p = 0.1f;
      viTri.y += 0.85f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 1.7f, 0.2f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      viTri.y += 0.45f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 1.7f, 1.0f, 0.7f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.0f;    mau.dd = 0.5f;
      viTri.y += 0.5f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 1.0f, 0.7f, 0.3f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      mau.d = 1.0f;   mau.l = 0.5f;   mau.x = 0.5f;    mau.dd = 1.0f;
      viTri.y += 0.5f;
      danhSachVat[chiSo].hinhDang.hinhNon = datHinhNon( 0.7f, 0.0f, 0.7f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      goc += buocGoc;
   }

   // ---- đấy trắng
   mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;    mau.dd = 1.0f;   mau.p = 0.0f;
   goc = buocGoc*1.5f;
   
   while( chiSo < 96 ) {  // 6 cái * 2 bộ phần
      viTri.x = 17.5f*cosf( goc ) - 52.0f;      viTri.z = 17.5f*sinf( goc ) - 52.0f;
      
      viTri.y = 13.19f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.3f, 0.58f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      viTri.y = 14.6f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.2f, 0.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      goc += buocGoc;
      chiSo++;
   }

   goc = 3.1415926f + buocGoc*1.5f;
   while( chiSo < 108 ) {  // 6 cái * 2 bộ phần
      viTri.x = 17.5f*cosf( goc ) - 52.0f;    viTri.z = 17.5f*sinf( goc ) - 52.0f;
      
      viTri.y = 13.19f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.3f, 0.58f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      
      viTri.y = 14.6f;
      danhSachVat[chiSo].hinhDang.hinhTru = datHinhTru( 2.2f, 0.5f, &(danhSachVat[chiSo].baoBiVT) );
      danhSachVat[chiSo].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSo].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      goc += buocGoc;
      chiSo++;
   }

   return chiSo;
}


unsigned short datCacCotVuongLong( VatThe *danhSachVat ) {
   
   Vecto vectoXoay;
   vectoXoay.x = 0.0f;    vectoXoay.y = 1.0f;     vectoXoay.z = 0.0f;
   Quaternion quaternion = datQuaternionTuVectoVaGocQuay( &vectoXoay, 0.0f );
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   Vecto viTri;
   
   // ----
   mau.d = 0.3f;   mau.l = 1.0f;   mau.x = 0.3f;    mau.dd = 0.2f;    mau.p = 0.8f;
   viTri.x = -15.0f;      viTri.y = 11.0f;      viTri.z = -10.0f;
   
   danhSachVat[0].hinhDang.hinhTru = datHinhTru( 1.5f, 1.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[0].chietSuat = 1.4f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.x = -10.0f;      viTri.y = 11.0f;      viTri.z = -15.0f;
   danhSachVat[1].hinhDang.hinhTru = datHinhTru( 1.5f, 1.0f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[1].chietSuat = 1.4f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[1].soHoaTiet = kHOA_TIET__KHONG;
   
   return 2;
}


unsigned short datKimTuThapThuyTinh( VatThe *danhSachVat ) {
 
   Vecto vectoXoay;
   vectoXoay.x = 0.0f;    vectoXoay.y = 1.0f;     vectoXoay.z = 0.0f;
   Quaternion quaternion = datQuaternionTuVectoVaGocQuay( &vectoXoay, 0.0f );
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   Vecto viTri;
   
   // ---- 
   mau.d = 0.3f;   mau.l = 0.3f;   mau.x = 0.3f;    mau.dd = 1.0f;   mau.p = 0.8f;
   viTri.x = 15.0f;      viTri.y = 8.5f;      viTri.z = 13.0f;

   danhSachVat[0].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[0].chietSuat = 1.4f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;

   unsigned short soHang = 0;
   unsigned short chiSo = 1;
   
   viTri.x = 14.25f;
   viTri.y += 0.75f;
   viTri.z = 14.25f;
   soHang = 0;
   while ( soHang < 2 ) {
      unsigned short soCot = 0;
      while (soCot < 2 ) {
         danhSachVat[chiSo].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
         danhSachVat[chiSo].loai = kLOAI_HINH_DANG__BAT_DIEN;
         danhSachVat[chiSo].chietSuat = 1.4f;
         datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
         danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
         danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
         viTri.x += 1.5f;
         chiSo++;  //
         soCot++;
      }
      viTri.x = 14.25f;
      viTri.z += 1.5f;
      soHang++;
   }

   viTri.x = 13.5f;
   viTri.y += 0.75f;
   viTri.z = 13.5f;
   soHang = 0;
   while ( soHang < 3 ) {
      unsigned short soCot = 0;
      while (soCot < 3 ) {
         danhSachVat[chiSo].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
         danhSachVat[chiSo].loai = kLOAI_HINH_DANG__BAT_DIEN;
         danhSachVat[chiSo].chietSuat = 1.4f;
         datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
         danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
         danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
         viTri.x += 1.5f;
         chiSo++;  //
         soCot++;
      }
      viTri.x = 13.5f;
      viTri.z += 1.5f;
      soHang++;
   }
   
   viTri.x = 12.75f;
   viTri.y += 0.75f;
   viTri.z = 12.75f;
   
   soHang = 0;
   while ( soHang < 4 ) {
      unsigned short soCot = 0;
      while (soCot < 4 ) {
         danhSachVat[chiSo].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
         danhSachVat[chiSo].loai = kLOAI_HINH_DANG__BAT_DIEN;
         danhSachVat[chiSo].chietSuat = 1.4f;
         datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
         danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
         danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
         viTri.x += 1.5f;
         chiSo++;  //
         soCot++;
      }
      viTri.x = 12.75f;
      viTri.z += 1.5f;
      soHang++;
   }

   viTri.x = 12.0f;
   viTri.y += 0.75f;
   viTri.z = 12.0f;
   soHang = 0;
   while ( soHang < 5 ) {
      unsigned short soCot = 0;
      while (soCot < 5 ) {
         danhSachVat[chiSo].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
         danhSachVat[chiSo].loai = kLOAI_HINH_DANG__BAT_DIEN;
         danhSachVat[chiSo].chietSuat = 1.4f;
         datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
         danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
         danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
         viTri.x += 1.5f;
         chiSo++;  // 
         soCot++;
      }
      viTri.x = 12.0f;
      viTri.z += 1.5f;
      soHang++;
   }

   viTri.x = 12.75f;
   viTri.y += 0.75f;
   viTri.z = 12.75f;
   soHang = 0;
   while ( soHang < 4 ) {
      unsigned short soCot = 0;
      while (soCot < 4 ) {
         danhSachVat[chiSo].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
         danhSachVat[chiSo].loai = kLOAI_HINH_DANG__BAT_DIEN;
         danhSachVat[chiSo].chietSuat = 1.4f;
         datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
         danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
         danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
         viTri.x += 1.5f;
         chiSo++;  //
         soCot++;
      }
      viTri.x = 12.75f;
      viTri.z += 1.5f;
      soHang++;
   }

   viTri.x = 13.5f;
   viTri.y += 0.75f;
   viTri.z = 13.5f;
   soHang = 0;
   while ( soHang < 3 ) {
      unsigned short soCot = 0;
      while (soCot < 3 ) {
         danhSachVat[chiSo].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
         danhSachVat[chiSo].loai = kLOAI_HINH_DANG__BAT_DIEN;
         danhSachVat[chiSo].chietSuat = 1.4f;
         datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
         danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
         danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
         viTri.x += 1.5f;
         chiSo++;  //
         soCot++;
      }
      viTri.x = 13.5f;
      viTri.z += 1.5f;
      soHang++;
   }

   viTri.x = 14.25f;
   viTri.y += 0.75f;
   viTri.z = 14.25f;
   soHang = 0;
   while ( soHang < 2 ) {
      unsigned short soCot = 0;
      while (soCot < 2 ) {
         danhSachVat[chiSo].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
         danhSachVat[chiSo].loai = kLOAI_HINH_DANG__BAT_DIEN;
         danhSachVat[chiSo].chietSuat = 1.4f;
         datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
         danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
         danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
         viTri.x += 1.5f;
         chiSo++;  //
         soCot++;
      }
      viTri.x = 14.25f;
      viTri.z += 1.5f;
      soHang++;
   }

   viTri.x = 15.0f;
   viTri.y += 0.75f;
   viTri.z = 15.0f;
   danhSachVat[chiSo].hinhDang.batDien = datBatDien( 1.5f, 1.5f, 1.5f, &(danhSachVat[chiSo].baoBiVT) );
   danhSachVat[chiSo].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[chiSo].chietSuat = 1.4f;
   datBienHoaChoVat( &(danhSachVat[chiSo]), &phongTo, &quaternion, &viTri );
   danhSachVat[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[chiSo].soHoaTiet = kHOA_TIET__KHONG;
   return 55 + 16 + 9 + 4 + 1;
}

unsigned short datDayHinhNonHuongX( VatThe *danhSachVat ) {
   
   Vecto truc;
   truc.x = 1.0f;    truc.y = 1.0f;    truc.z = 0.0f;
   Quaternion xoay;
   xoay.w = 1.0f;   xoay.x = 0.0f;    xoay.y = 0.0f;    xoay.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   
   unsigned short chiSo = 0;
   unsigned short chiSoVathThe = 0;
   while ( chiSo < 15 ) {
      Vecto tam;
      tam.x = -20.0f - chiSo*35.0f;
      tam.y = 10.2f;
      tam.z = 15.0f;
      Mau mau;
      //      mau.d = 1.0f;   mau.l = 0.3f;   mau.x = 0.3f;   mau.dd = 1.0f;   mau.p = 0.3f;
      mau.d = 0.7f;   mau.l = 1.0f;   mau.x = 1.0f;   mau.dd = 1.0f;   mau.p = 0.3f;
      // ---- nền
      danhSachVat[chiSoVathThe].hinhDang.hop = datHop( 5.0f, 0.4f, 5.0f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;

      // ---- đấy
      //     mau.d = 1.0f;   mau.l = 0.7f;   mau.x = 0.7f;   mau.dd = 1.0f;   mau.p = 0.3f;
      mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;   mau.dd = 1.0f;   mau.p = 0.3f;
      tam.y += 0.3f;
      danhSachVat[chiSoVathThe].hinhDang.hinhTru = datHinhTru( 2.3f, 0.2f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;

      tam.y += 0.6f;
      danhSachVat[chiSoVathThe].hinhDang.hinhNon = datHinhNon( 2.3f, 1.3f, 1.0f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;

      // ---- phần giữa
      //      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.3f;   mau.dd = 1.0f;   mau.p = 0.3f;
      mau.d = 0.0f;   mau.l = 1.0f;   mau.x = 0.3f;   mau.dd = 1.0f;   mau.p = 0.3f;
      tam.y += 0.6f;
      danhSachVat[chiSoVathThe].hinhDang.hinhTru = datHinhTru( 1.1f, 0.2f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;

      tam.y += 0.3f;
      danhSachVat[chiSoVathThe].hinhDang.hinhNon = datHinhNon( 1.1f, 0.7f, 0.4f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;

      // ---- nắp
      tam.y += 0.3f;
      //      mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.3f;   mau.dd = 0.4f;   mau.p = 0.3f;
      mau.d = 0.0f;   mau.l = 0.0f;   mau.x = 0.9f;   mau.dd = 1.0f;   mau.p = 0.8f;
      danhSachVat[chiSoVathThe].hinhDang.hinhTru = datHinhTru( 0.5f, 0.2f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSoVathThe].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;
   
      tam.y += 0.35f;
//      mau.d = 1.0f;   mau.l = 0.8f;   mau.x = 0.9f;   mau.dd = 1.0f;
      danhSachVat[chiSoVathThe].hinhDang.hinhNon = datHinhNon( 0.5f, 0.0f, 0.5f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;

      chiSo++;
   }
   
   return chiSoVathThe;
}


unsigned short datDayHinhNonHuongZ( VatThe *danhSachVat ) {
   
   Vecto truc;
   truc.x = 1.0f;    truc.y = 1.0f;    truc.z = 0.0f;
   Quaternion xoay;
   xoay.w = 1.0f;   xoay.x = 0.0f;    xoay.y = 0.0f;    xoay.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   
   unsigned short chiSo = 0;
   unsigned short chiSoVathThe = 0;
   while ( chiSo < 20 ) {
      Vecto tam;
      tam.x = 15.0f;
      tam.y = 10.2f;
      tam.z = -20.0f - chiSo*60.0f;
      Mau mau;
      mau.d = 0.5f;   mau.l = 1.0f;   mau.x = 0.3f;   mau.dd = 1.0f;   mau.p = 0.3f;
      // ---- nền
      danhSachVat[chiSoVathThe].hinhDang.hop = datHop( 5.0f, 0.4f, 5.0f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;
      
      // ---- đấy
      mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;

      tam.y += 0.3f;
      danhSachVat[chiSoVathThe].hinhDang.hinhTru = datHinhTru( 2.3f, 0.2f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;

      tam.y += 0.6f;
      danhSachVat[chiSoVathThe].hinhDang.hinhNon = datHinhNon( 2.3f, 1.3f, 1.0f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;
      
      // ---- phần giữa
      mau.d = 0.0f;   mau.l = 0.8f;   mau.x = 1.0f;
      tam.y += 0.6f;
      danhSachVat[chiSoVathThe].hinhDang.hinhTru = datHinhTru( 1.1f, 0.2f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_TRU;

      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;
      
      tam.y += 0.3f;
      danhSachVat[chiSoVathThe].hinhDang.hinhNon = datHinhNon( 1.1f, 0.7f, 0.4f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;
      
      // ---- nắp
      tam.y += 0.3f;
      mau.d = 0.5f;   mau.l = 0.0f;   mau.x = 1.0f;   mau.dd = 1.0f;   mau.p = 0.8f;
      danhSachVat[chiSoVathThe].hinhDang.hinhTru = datHinhTru( 0.5f, 0.2f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[chiSoVathThe].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;
      
      tam.y += 0.35f;
      //      mau.d = 1.0f;   mau.l = 0.8f;   mau.x = 0.9f;   mau.dd = 1.0f;   mau.p = 0.8f;
      danhSachVat[chiSoVathThe].hinhDang.hinhNon = datHinhNon( 0.5f, 0.0f, 0.5f, &(danhSachVat[chiSoVathThe].baoBiVT) );
      danhSachVat[chiSoVathThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[chiSoVathThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoVathThe]), &phongTo, &xoay, &tam );
      danhSachVat[chiSoVathThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[chiSoVathThe].soHoaTiet = kHOA_TIET__KHONG;
      chiSoVathThe++;
      
      chiSo++;
   }
   
   return chiSoVathThe;
}



#pragma mark ---- Nhân Vật
unsigned short datKienLongSaoGai( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   Vecto viTri;
   
   // ---- kiến lồng - vật thể ghép
   mau.d = 0.4f;      mau.l = 0.7f;      mau.x = 1.0f;      mau.dd = 0.15f;   mau.p = 0.1f;
   viTri.x = -52.0f;      viTri.y = 27.7f;        viTri.z = -52.0f;
//   danhSachVat[0].hinhDang.hinhCau = datHinhCau( 5.0f );  // cho xem vị trí tâm
   danhSachVat[0].loai = kLOAI_VAT_THE__GHEP;
   danhSachVat[0].chietSuat = 1.4f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   danhSachVat[0].soLuongVatThe = 20;
   danhSachVat[0].danhSachVatThe = malloc( 20*sizeof(VatThe) );

   // ---- các thành phân trong vật ghép
   VatThe *danhSachVatThe = danhSachVat[0].danhSachVatThe;
   Vecto trucQuay;
   trucQuay.x = 0.0f;
   trucQuay.y = 1.0f;
   trucQuay.z = 0.0f;
   float buocGoc = 6.283184f/20.0;    // 2π/20
   
   unsigned char chiSo = 0;
   while ( chiSo < 20 ) {
      float gocXoay = buocGoc*chiSo ;
      // ---- tính vị trí tương đối
      viTri.x = 3.3f*sinf( gocXoay );
      viTri.y = 0.0f;
      viTri.z = 3.3f*cosf( gocXoay );
      
      quaternion = datQuaternionTuVectoVaGocQuay( &trucQuay,  gocXoay );
      danhSachVatThe[chiSo].hinhDang.hop = datHop( 1.0f, 3.5f, 0.2f, &(danhSachVatThe[chiSo].baoBiVT) );
      danhSachVatThe[chiSo].loai = kLOAI_HINH_DANG__HOP;
      danhSachVatThe[chiSo].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachVatThe[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachVatThe[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVatThe[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
   }

   // ---- đừng quên tính bao bì vật thể cho vật thể ghép
   tinhBaoBiVTChoVatTheGhep( &(danhSachVat[0]) );

   return 1;
}

unsigned short datHinhNonBayQuanhThap( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   mau.d = 0.3f;   mau.l = 0.0f;   mau.x = 0.2f;    mau.dd = 0.2f;   mau.p = 0.1f;
   Vecto viTri;
   viTri.x = 0.0f;        viTri.y = 16.5f;     viTri.z = 0.0f;
   
   danhSachVat[0].loai = kLOAI_VAT_THE__GHEP;
   danhSachVat[0].chietSuat = 1.4f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   danhSachVat[0].soLuongVatThe = 16;
   danhSachVat[0].danhSachVatThe = malloc( 16*sizeof(VatThe) );
   
   // ---- các thành phân trong vật ghép
   VatThe *danhSachBoPhan = danhSachVat[0].danhSachVatThe;

   float buocGoc = 6.283184f/8.0f;
   float goc = 0;

   unsigned short chiSo = 0;
   while( chiSo < 16 ) {
      // ---- tính vị trí tương đối
      viTri.x = 3.3f*sinf( goc );  // 2π/20
      viTri.y = sinf( goc );
      viTri.z = 3.3f*cosf( goc );
      // ---- đấy
      danhSachBoPhan[chiSo].hinhDang.hinhNon = datHinhNon( 0.0f, 0.5f, 0.5f, &(danhSachBoPhan[chiSo].baoBiVT) );
      danhSachBoPhan[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachBoPhan[chiSo].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachBoPhan[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachBoPhan[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachBoPhan[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      chiSo++;
      // ---- nắp
      viTri.y += 1.0f;
      danhSachBoPhan[chiSo].hinhDang.hinhNon = datHinhNon( 0.5f, 0.0f, 1.5f, &(danhSachBoPhan[chiSo].baoBiVT) );
      danhSachBoPhan[chiSo].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachBoPhan[chiSo].chietSuat = 1.4f;
      datBienHoaChoVat( &(danhSachBoPhan[chiSo]), &phongTo, &quaternion, &viTri );
      danhSachBoPhan[chiSo].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachBoPhan[chiSo].soHoaTiet = kHOA_TIET__KHONG;
      // ---- tiếp
      goc += buocGoc;
      chiSo++;
   }
   
   // ---- đừng quên tính bao bì vật thể cho vật thể ghép
   tinhBaoBiVTChoVatTheGhep( &(danhSachVat[0]) );

   return 1;
}

unsigned short datKeThuSaoGai( VatThe *danhSachVat, Vecto viTri ) {
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   
   // ---- đấy - lớp 0
   mau.d = 0.8f;      mau.l = 0.0f;      mau.x = 0.0f;      mau.dd = 1.0f;   mau.p = 0.2f;
   danhSachVat[0].hinhDang.saoGai = datSaoGai( &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__SAO_GAI;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   danhSachVat[0].thaiTrang = 0;
   
   return 1;
}


unsigned short datTraiBanhBiGiet( VatThe *danhSachVat ) {
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   Vecto viTri;
   
   float banKinh = 2.2f;
   
   Mau mauNen;
   mauNen.d = 1.0f;      mauNen.l = 1.0f;      mauNen.x = 1.0f;      mauNen.dd = 1.0f;   mauNen.p = 0.2f;
   Mau mauOc0;
   mauOc0.d = 1.0f;      mauOc0.l = 0.3f;      mauOc0.x = 0.3f;      mauOc0.dd = 1.0f;   mauOc0.p = 0.2f;
   Mau mauOc1;
   mauOc1.d = 0.5f;      mauOc1.l = 0.5f;      mauOc1.x = 0.5f;      mauOc1.dd = 1.0f;   mauOc1.p = 0.2f;
   Mau mauOc2;
   mauOc2.d = 0.5f;      mauOc2.l = 1.0f;      mauOc2.x = 0.5f;      mauOc2.dd = 1.0f;   mauOc2.p = 0.2f;
   
   // ---- thật vị trí không quan trọn
   viTri.x = -3.0f;      viTri.y = 10.1 + banKinh;        viTri.z = -523.0f;
   danhSachVat[0].hinhDang.hinhCau = datHinhCau( banKinh, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mauNen, &mauOc0, &mauOc1, &mauOc2, 0.05f, 0.08f, 0.15f, 0.0f, 0.165f, 1 );
   danhSachVat[0].soHoaTiet = kHOA_TIET__QUAN_XOAY;
   
   danhSachVat[0].thaiTrang = 0;
   
   return 1;
}


#define kDUONG_X  0
#define kDUONG_Z  1
#define kTHAI_TRANG__VAO_VONG    0
#define kTHAI_TRANG__TRONG_VONG  1
#define kTHAI_TRANG__RA_VONG     2

unsigned short datCacTraiBanhTrenLoHuongX( VatThe *danhSachVat ) {
   
   float danhSachViTri[] = {-10.0f, -14.0f, -18.0f, -34.0f, -37.0,
       -40.0f,  -43.0f,  -48.0f,  -52.0f,  -74.0f,  // 10
       -82.0f,  -90.0f,  -94.0f,  -98.0f,  -105.5f,
      -109.0f, -112.5f, -121.0f, -123.5f, -126.0f,  // 20
      -128.5f, -147.5f, -152.5f, -157.5f, -169.5f,
       -172.5f, -176.5f, -180.0f, -183.5f, -195.0f, // 30
       -198.5f, -219.0f, -222.5f, -226.0f, -234.0f,
       -238.0f, -248.0f, -260.0f, -290.0f, -296.0f, // 40
      
      -302.0f, -312.5f, -316.0f, -351.0f, -356.0f,
      -359.0f, -363.0f, -372.0f, -376.0f, -410.0f,  // 50
   
      -415.0f, -420.0f, -425.0f, -460.0f, -465.0f,
      -469.0f, -472.0f, -475.0f, -478.0f, -481.0f,  // 60
      -506.0f, -509.0f, -512.0f, -520.0f, -523.5f,
      -569.0f, -575.0f, -580.0f, -583.5f, -587.0f,  // 70
      -600.0f, -605.0f, -609.0f, -615.0f, -620.0f, 
   };
   
   float danhSachBanKinh[] = { 1.50f, 1.50f, 1.50f, 1.00f, 1.00f,
      1.00f, 1.00f, 0.95f, 0.95f, 1.40f,  // 10
      1.50f, 0.80f, 0.80f, 0.80f, 1.20f,
      1.20f, 1.20f, 1.00f, 1.00f, 1.00f,  // 20
      1.00f, 1.70f, 1.70f, 1.70f, 1.10f,
      1.10f, 1.30f, 1.30f, 1.30f, 1.25f,  // 30
      1.25f, 1.35f, 1.35f, 1.35f, 1.50f,
      1.50f, 1.80f, 1.80f, 2.50f, 2.50f,  // 40
     
      2.50f, 1.30f, 1.30f, 2.00f, 1.10f,
      1.10f, 1.10f, 1.10f, 1.10f, 1.35f,  // 50
   
      1.35f, 1.35f, 1.35f, 1.60f, 1.60f,
      1.15f, 1.15f, 1.15f, 1.15f, 1.15f,  // 60
      1.15f, 1.15f, 1.15f, 0.95f, 1.55f,
      2.05f, 2.05f, 1.05f, 1.05f, 1.05f,  // 79
      0.95f, 1.35f, 1.35f, 1.65f, 1.65f,
   };

   float danhSachDuongRa[] = { kDUONG_X, kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X, // 10
      kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X,
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_X, // 20
      kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X,
      kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X, // 30
      kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X,
      kDUONG_X, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X, // 40

      kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X,
      kDUONG_Z, kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_X, // 50
   
      kDUONG_X, kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, // 60
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X, // 70
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_X, kDUONG_X,
   };


   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 1.0f;  quaternion.y = 1.0f;  quaternion.z = 1.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   
   unsigned short chiSoTraiBanh = 0;
   unsigned short soLuongTraiBanh = 75;
   while( chiSoTraiBanh < soLuongTraiBanh ) {
      // ---- bán kính
      float banKinh = danhSachBanKinh[chiSoTraiBanh];
      // --- vị trí
      viTri.x = danhSachViTri[chiSoTraiBanh];
      viTri.y = 10.1f + banKinh;
      viTri.z = 3.0f;

      danhSachVat[chiSoTraiBanh].duongVao = kDUONG_X;
      danhSachVat[chiSoTraiBanh].duongRa = danhSachDuongRa[chiSoTraiBanh];
      danhSachVat[chiSoTraiBanh].hinhDang.hinhCau = datHinhCau( banKinh, &(danhSachVat[chiSoTraiBanh].baoBiVT) );
      danhSachVat[chiSoTraiBanh].loai = kLOAI_HINH_DANG__HINH_CAU;
      danhSachVat[chiSoTraiBanh].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoTraiBanh]), &phongTo, &quaternion, &viTri );
      // ---- chọn họa tiết và cỡ kích trái banh
      chonVaToHoatTietNgauNhien( &(danhSachVat[chiSoTraiBanh].soHoaTiet), &(danhSachVat[chiSoTraiBanh].hoaTiet) );

      chiSoTraiBanh++;
   }

   // ---- CÁC TRÁI BANH ĐANG CHẠY RA khi bắt đầu mô phỏng
   float danhSachViTriChayRa[] = { -14.0f, -17.0f, -20.0f, -30.0f, -32.5f,
      -35.0f, -37.5f, -40.0f, -58.0f, -62.0f,  // 10
      -72.0f, -74.0f, -78.0f, -81.0f, -84.0f,
      -90.0f, -92.0f, -95.0f, -97.5f, -100.0f,  // 20
   };
   
   float danhSachBanKinhChayRa[] = { 1.25f, 1.25f, 1.25f, 1.00f, 1.00f,
      1.00f, 1.00f, 1.00f, 1.95f, 1.95f,  // 10
      1.50f, 1.50f, 1.70f, 1.70f, 1.70f,
      0.90f, 0.90f, 1.25f, 1.25f, 1.25f,  // 20
   };

   unsigned short soLuongTraiBanhTruoc = chiSoTraiBanh;
   soLuongTraiBanh = chiSoTraiBanh + 20;
   while( chiSoTraiBanh < soLuongTraiBanh ) {
      // ---- bán kính
      float banKinh = danhSachBanKinhChayRa[chiSoTraiBanh - soLuongTraiBanhTruoc];
      // --- vị trí
      viTri.x = danhSachViTriChayRa[chiSoTraiBanh - soLuongTraiBanhTruoc];
      viTri.y = 10.1f + banKinh;
      viTri.z = -3.0f;

      danhSachVat[chiSoTraiBanh].duongVao = kDUONG_X;
      danhSachVat[chiSoTraiBanh].duongRa = kDUONG_X; // đang chạy ra đường này
      danhSachVat[chiSoTraiBanh].hinhDang.hinhCau = datHinhCau( banKinh, &(danhSachVat[chiSoTraiBanh].baoBiVT) );
      danhSachVat[chiSoTraiBanh].loai = kLOAI_HINH_DANG__HINH_CAU;
      danhSachVat[chiSoTraiBanh].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoTraiBanh]), &phongTo, &quaternion, &viTri );
      // ---- trái banh này -ang đi ra ngoài
      danhSachVat[chiSoTraiBanh].thaiTrang = kTHAI_TRANG__RA_VONG;
      // ---- chọn họa tiết và cỡ kích trái banh
      chonVaToHoatTietNgauNhien( &(danhSachVat[chiSoTraiBanh].soHoaTiet), &(danhSachVat[chiSoTraiBanh].hoaTiet) );
      
      chiSoTraiBanh++;
   }
   
   return chiSoTraiBanh;
}

unsigned short datCacTraiBanhTrenLoHuongZ( VatThe *danhSachVat ) {
   
   float danhSachViTri[] = {-12.0f, -15.0f, -18.0f, -21.0f, -24.5f,
      -28.5f,  -32.0f,  -35.5f,   -39.0f,  -42.5f,  // 10
      -46.5f,  -52.0f,  -71.0f,   -73.5f,  -76.0f,
      -78.5f,  -81.0f,  -103.0f, -107.0f, -111.0f,  // 20
      -124.0f, -132.0f, -136.0f, -186.0f, -190.0f,
      -194.0f, -198.0,   -202.0f, -225.0f, -229.0,  // 30
      -250.0f, -254.0f, -258.0f, -262.0f,  -266.0,
      -270.0f, -295.0f, -298.0f, -301.0f,  -330.0,  // 40
      -356.0f, -359.0f, -362.0f, -377.0f,  -382.0f,
      -386.0f, -390.0f, -395.0f, -399.0f,  -414.0f, // 50
      -417.5f, -425.0f, -429.0f, -433.0f,  -437.0f,
      -441.0f, -445.0f, -449.0f, -480.0f, -484.0f,  // 60
      -488.0f, -492.0f, -496.0f, -502.0f, -508.0f,
      -516.0f, -519.0f, -522.0f, -525.0f, -575.0f,  // 70
      -624.0f, -640.0f, -648.0f, -660.0f, -663.0f,
      -667.0f, -671.0f, -696.0f, -699.0f, -702.0f,  // 80
      -714.0f, -745.0f, -748.5f, -755.0f, -759.0f,
      -763.0f, -767.0f, -784.0f, -792.0f, -796.0f,  // 90
      -802.0f, -806.0f, -810.0f, -814.0f, -818.0,
      -822.0f, -835.0f, -838.0,  -850.0f, -854.0f,  // 100
      -858.0f, -862.0f, -866.0, -874.0f, -895.0f,
      -898.0f, -901.0f, -912.0f, -915.0f, -918.0f,  // 110
      -921.0f, -924.5f, -928.5f,  -932.0f,  -935.5f,
      -939.0f,  -942.5f, -946.5f,  -952.0f,  -971.0f,  // 120
      -973.5f,  -976.0f, -978.5f,  -981.0f,  -1003.0f,
      -1007.0f, -1011.0f, -1015.0f, -1025.0f, -1028.0f,  // 130
      -1033.0f, -1047.5f, -1051.5f, -1056.0f, -1060.0f,
      -1065.0f, -1069.0f, -1076.0f, -1082.0f, -1086.0f,  // 140
      -1090.0f, -1094.0f, -1099.0f, -1104.0f, -1109.0f,
      -1117.0f, -1121.0f, -1181.0f, -1185.0f, -1188.0f,  // 150
      -1193.0f, -1197.5f, -1201.5f, -1206.0f, -1210.0f,
      -1215.0f, -1219.0f, -1236.0f, -1242.0f, -1246.0f,  // 160
      -1250.0f, -1254.0f, -1269.0f, -1274.0f, -1279.0f,
      -1287.0f, -1291.0f, -1304.0f, -1312.0f, -1316.0f,  // 170
      -1322.0f, -1326.0f, -1330.0f, -1334.0f, -1338.0,
      -1342.0f, -1345.0f, -1349.0,  -1353.0f, -1364.0f,  // 180
      -1378.0f, -1382.0f,  -1386.0, -1390.0f, -1395.0f,
      -1399.0f, -1401.0f, -1409.0f, -1413.0f, -1419.0f, // 190
   };
   
float danhSachBanKinh[] = { 1.10f, 1.10f, 1.10f, 1.10f, 1.10f,
      1.40f, 1.40f, 1.40f, 1.40f, 1.40f,   // 10
      1.80f, 1.80f, 0.90f, 0.90f, 0.90f,
      0.95f, 0.95f, 1.50f, 1.50f, 1.50f,   // 20
      1.23f, 1.70f, 1.30f, 1.50f, 1.50f,
      1.80f, 1.80f, 1.50f, 1.50f, 1.50f,   // 30
      1.50f, 1.50f, 1.50f, 1.50f, 1.50f,
      2.00f, 1.35f, 1.35f, 1.35f, 2.50f,   // 40
      1.00f, 1.00f, 1.00f, 2.00f, 1.70f,
      1.40f, 1.10f, 1.80f, 1.80f, 1.80f,   // 50
      1.05f, 1.70f, 1.70f, 1.70f, 1.50f,
      1.50f, 1.50f, 1.50f, 1.70f, 1.50f,   // 60
      1.50f, 1.50f, 1.50f, 2.00f, 2.00f,
      1.10f, 1.10f, 1.10f, 1.10f, 2.20f,   // 70
      1.30f, 1.30f, 2.10f, 1.20f, 1.20f,
      1.50f, 1.50f, 1.10f, 1.10f, 1.10f,   // 80
      1.80f, 1.25f, 1.25f, 1.50f, 1.50f,
      1.50f, 1.50f, 1.23f, 1.70f, 1.30f,   // 90
      1.50f, 1.50f, 1.50f, 1.80f, 1.80f,
      1.50f, 1.50f, 1.50f, 1.50f, 1.50f,   // 100
      1.50f, 1.50f, 1.50f, 2.00f, 1.35f,
      1.35f, 1.35f, 1.10f, 1.10f, 1.10f,   // 110
      1.10f, 1.10f, 1.40f, 1.40f, 1.40f,
      1.40f, 1.40f, 1.80f, 1.80f, 0.90f,   // 120
      0.90f, 0.90f, 0.95f, 0.95f, 1.50f,
      1.50f, 1.50f, 1.30f, 1.30f, 1.30f,   // 130
      1.00f, 1.00f, 1.45f, 1.25f, 1.80f,
      1.25f, 1.25f, 1.50f, 2.00f, 1.35f,   // 140
      1.35f, 1.35f, 2.50f, 1.00f, 1.00f,
      1.00f, 2.00f, 1.30f, 1.30f, 1.30f,   // 150
      1.00f, 1.00f, 1.45f, 1.25f, 1.80f,
      1.25f, 1.25f, 1.50f, 2.00f, 1.35f,   // 160
      1.35f, 1.35f, 2.50f, 1.00f, 1.00f,
      1.00f, 2.00f, 1.23f, 1.70f, 1.30f,   // 170
      1.50f, 1.50f, 1.50f, 1.80f, 1.80f,
      1.50f, 1.50f, 1.50f, 1.50f, 1.50f,   // 180
      1.50f, 1.50f, 1.50f, 2.00f, 1.35f,
      1.35f, 1.35f, 1.05f, 1.05f, 1.05f,   // 190
      1.50f, 1.50f, 1.23f, 1.70f, 1.30f,
      1.45f, 1.45f, 1.05f, 1.80f, 1.80f,   // 200
      };

float danhSachDuongRa[] = { kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,
      kDUONG_X, kDUONG_X, kDUONG_X, kDUONG_X, kDUONG_X,  // 10
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X,  // 20
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X,
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X,  // 30
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,
      kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,  // 40
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X,  // 50
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X,  // 60
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X,
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X,  // 70
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X,  // 80
      kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_X,
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z,  // 90
      kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X, kDUONG_X,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_Z, kDUONG_Z,  // 100
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,  // 110
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X,
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z,  // 120
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X,
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z,  // 130
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_X,
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_X, kDUONG_Z,  // 140
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,  // 150
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_X,
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_X, kDUONG_Z,  // 160
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_Z,  // 170
      kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X, kDUONG_X,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_Z, kDUONG_Z,  // 180
      kDUONG_Z, kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X, // 190
      kDUONG_X, kDUONG_X, kDUONG_Z, kDUONG_Z, kDUONG_Z,
      kDUONG_Z, kDUONG_Z, kDUONG_X, kDUONG_X, kDUONG_X,  // 200
   };

   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 1.0f;  quaternion.y = 1.0f;  quaternion.z = 1.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   
   unsigned short chiSoTraiBanh = 0;
   unsigned short soLuongTraiBanh = 187;
   while( chiSoTraiBanh < soLuongTraiBanh ) {
      // ---- chọn bán kính
      float banKinh = danhSachBanKinh[chiSoTraiBanh];

      viTri.x = -3.0f;
      viTri.y = 10.1f + banKinh;
      viTri.z = danhSachViTri[chiSoTraiBanh];
      // ---- chọn vị trí cho các trái banh

      danhSachVat[chiSoTraiBanh].duongVao = kDUONG_Z;
      danhSachVat[chiSoTraiBanh].duongRa = danhSachDuongRa[chiSoTraiBanh];
      danhSachVat[chiSoTraiBanh].hinhDang.hinhCau = datHinhCau( banKinh, &(danhSachVat[chiSoTraiBanh].baoBiVT) );
      danhSachVat[chiSoTraiBanh].loai = kLOAI_HINH_DANG__HINH_CAU;
      danhSachVat[chiSoTraiBanh].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoTraiBanh]), &phongTo, &quaternion, &viTri );
      
      // ---- chọn họa tiết và cỡ kích trái banh
      chonVaToHoatTietNgauNhien( &(danhSachVat[chiSoTraiBanh].soHoaTiet), &(danhSachVat[chiSoTraiBanh].hoaTiet) );
      
      chiSoTraiBanh++;
   }
   
   // ---- CÁC TRÁI BANH ĐANG CHẠY RA khi bắt đầu mô phỏng
   float danhSachViTriChayRa[] = { -24.0f, -26.0f, -31.0f, -35.0f, -38.0f,
      -45.0f, -47.5f, -52.5f, -57.0f, -62.0f,  // 10
      -68.0f, -71.0f, -74.0f, -82.0f, -86.0f,
      -90.0f, -94.0f, -100.0f, -105.0f, -108.0f,  // 20
   };
   
   float danhSachBanKinhChayRa[] = { 0.90f, 0.90f, 1.50f, 1.30f, 1.30f,
      1.00f, 1.00f, 1.00f, 1.95f, 1.95f,  // 10
      1.25f, 1.25f, 1.25f, 1.75f, 1.75f,
      1.75f, 1.75f, 2.35, 1.35f, 1.35f,   // 20
   };
   
   unsigned short soLuongTraiBanhTruoc = chiSoTraiBanh;
   soLuongTraiBanh = chiSoTraiBanh + 20;

   while( chiSoTraiBanh < soLuongTraiBanh ) {
      // ---- chọn bán kính
      float banKinh = danhSachBanKinhChayRa[chiSoTraiBanh - soLuongTraiBanhTruoc];
      
      viTri.x = 3.0f;
      viTri.y = 10.1f + banKinh;
      viTri.z = danhSachViTriChayRa[chiSoTraiBanh - soLuongTraiBanhTruoc];
      // ---- chọn vị trí cho các trái banh

      danhSachVat[chiSoTraiBanh].duongVao = kDUONG_Z;
      danhSachVat[chiSoTraiBanh].duongRa = kDUONG_Z;
      danhSachVat[chiSoTraiBanh].hinhDang.hinhCau = datHinhCau( banKinh, &(danhSachVat[chiSoTraiBanh].baoBiVT) );
      danhSachVat[chiSoTraiBanh].loai = kLOAI_HINH_DANG__HINH_CAU;
      danhSachVat[chiSoTraiBanh].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[chiSoTraiBanh]), &phongTo, &quaternion, &viTri );
      // ---- đặt thại trạng trái banh này đi ra ngoài
      danhSachVat[chiSoTraiBanh].thaiTrang = kTHAI_TRANG__RA_VONG;
      
      // ---- chọn họa tiết và cỡ kích trái banh
      chonVaToHoatTietNgauNhien( &(danhSachVat[chiSoTraiBanh].soHoaTiet), &(danhSachVat[chiSoTraiBanh].hoaTiet) );
      
      chiSoTraiBanh++;
   }

   return chiSoTraiBanh;
}


unsigned short datVaiChanh( VatThe *danhSachVat, Vecto viTri ) {

   Vecto truc;
   truc.x = 1.0f;    truc.y = 1.0f;    truc.z = 0.0f;
   Quaternion quaternion = datQuaternionTuVectoVaGocQuay( &truc, 1.570795f );
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   danhSachVat[0].hinhDang.hinhCau = datHinhCau( 0.7f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
//   danhSachVat[0].hoaTiet.hoaTietKhong = kHOA_TIET__TRAI_BANH;
   danhSachVat[0].soHoaTiet = kHOA_TIET__TRAI_BANH;
   
   return 1;
}


unsigned short datBongBong( VatThe *danhSachVat ) {
 
   Vecto truc;
   truc.x = 1.0f;    truc.y = 1.0f;    truc.z = 0.0f;
   Quaternion xoay;
   xoay.w = 1.0f;   xoay.x = 0.0f;    xoay.y = 0.0f;    xoay.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   Mau mauRanh;
   Mau mauTam;

   // ---- trái banh vai chánh
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;    mauRanh.dd = 0.2f;   mauRanh.p = 0.3f;
   mauTam.d = 0.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;    mauTam.dd = 0.0f;   mauTam.p = 0.3f;
   viTri.x = -4.0f;  viTri.y = 5.0f; viTri.z = 470.0f;
   danhSachVat[0].hinhDang.hinhCau = datHinhCau( 0.7f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &xoay, &viTri );
   danhSachVat[0].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[0].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[0].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.8f;    mauRanh.x = 1.0f;
   mauTam.d = 0.5f;    mauTam.l = 1.0f;    mauTam.x = 0.0f;
   viTri.x = -4.8f;  viTri.y = 4.3f; viTri.z = 472.0f;
   danhSachVat[1].hinhDang.hinhCau = datHinhCau( 0.25f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &xoay, &viTri );
   danhSachVat[1].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[1].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[1].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.5f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 0.3f;
   viTri.x = -8.8f;  viTri.y = 3.3f; viTri.z = 465.0f;
   danhSachVat[2].hinhDang.hinhCau = datHinhCau( 0.3f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &xoay, &viTri );
   danhSachVat[2].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[2].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[2].viTriDau = viTri;

   // ----- độc lập
   mauRanh.d = 1.0f;    mauRanh.l = 0.8f;    mauRanh.x = 1.0f;
   mauTam.d = 0.5f;    mauTam.l = 1.0f;    mauTam.x = 0.0f;
   viTri.x = -18.0f;  viTri.y = 6.0f; viTri.z = 445.0f;
   danhSachVat[3].hinhDang.hinhCau = datHinhCau( 0.2f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[3].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &xoay, &viTri );
   danhSachVat[3].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[3].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[3].viTriDau = viTri;

   // ---- nhóm
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.3f;    mauTam.x = 1.0f;
   viTri.x = -65.0f;  viTri.y = 10.0f; viTri.z = 420.0f;
   danhSachVat[4].hinhDang.hinhCau = datHinhCau( 0.5f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[4].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &xoay, &viTri );
   danhSachVat[4].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[4].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[4].viTriDau = viTri;

   mauRanh.d = 0.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.3f;    mauTam.x = 0.0f;
   viTri.x = -70.0f;  viTri.y = 5.0f; viTri.z = 413.0f;
   danhSachVat[5].hinhDang.hinhCau = datHinhCau( 0.8f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[5].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &xoay, &viTri );
   danhSachVat[5].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[5].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[5].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 0.0f;
   mauTam.d = 1.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -68.5f;  viTri.y = 5.5f; viTri.z = 410.0f;
   danhSachVat[6].hinhDang.hinhCau = datHinhCau( 0.2f, &(danhSachVat[6].baoBiVT) );
   danhSachVat[6].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[6].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6]), &phongTo, &xoay, &viTri );
   danhSachVat[6].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[6].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[6].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -68.0f;  viTri.y = 6.0f; viTri.z = 407.0f;
   danhSachVat[7].hinhDang.hinhCau = datHinhCau( 0.7f, &(danhSachVat[7].baoBiVT) );
   danhSachVat[7].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[7].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[7]), &phongTo, &xoay, &viTri );
   danhSachVat[7].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[7].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[7].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -72.0f;  viTri.y = 7.5f; viTri.z = 407.0f;
   danhSachVat[8].hinhDang.hinhCau = datHinhCau( 0.5f, &(danhSachVat[8].baoBiVT) );
   danhSachVat[8].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[8].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[8]), &phongTo, &xoay, &viTri );
   danhSachVat[8].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[8].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[8].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.7f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.5f;    mauTam.x = 1.0f;
   viTri.x = -74.0f;  viTri.y = 5.5f; viTri.z = 405.0f;
   danhSachVat[9].hinhDang.hinhCau = datHinhCau( 0.1f, &(danhSachVat[9].baoBiVT) );
   danhSachVat[9].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[9].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[9]), &phongTo, &xoay, &viTri );
   danhSachVat[9].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[9].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[9].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.3f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -85.0f;  viTri.y = 5.5f; viTri.z = 380.0f;
   danhSachVat[10].hinhDang.hinhCau = datHinhCau( 0.21f, &(danhSachVat[10].baoBiVT) );
   danhSachVat[10].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[10].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[10]), &phongTo, &xoay, &viTri );
   danhSachVat[10].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[10].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[10].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 0.0f;
   viTri.x = -88.0f;  viTri.y = 4.0f; viTri.z = 372.0f;
   danhSachVat[11].hinhDang.hinhCau = datHinhCau( 0.5f, &(danhSachVat[11].baoBiVT) );
   danhSachVat[11].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[11].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[11]), &phongTo, &xoay, &viTri );
   danhSachVat[11].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[11].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[11].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.5f;    mauTam.l = 1.0f;    mauTam.x = 0.5f;
   viTri.x = -90.0f;  viTri.y = 6.0f; viTri.z = 372.0f;
   danhSachVat[12].hinhDang.hinhCau = datHinhCau( 0.32f, &(danhSachVat[12].baoBiVT) );
   danhSachVat[12].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[12].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[12]), &phongTo, &xoay, &viTri );
   danhSachVat[12].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[12].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[12].viTriDau = viTri;

   mauRanh.d = 0.0f;    mauRanh.l = 1.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -102.0f;  viTri.y = 4.0f; viTri.z = 372.0f;
   danhSachVat[13].hinhDang.hinhCau = datHinhCau( 0.4f, &(danhSachVat[13].baoBiVT) );
   danhSachVat[13].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[13].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[13]), &phongTo, &xoay, &viTri );
   danhSachVat[13].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[13].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[13].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.8f;    mauRanh.x = 0.8f;
   mauTam.d = 0.0f;    mauTam.l = 0.3f;    mauTam.x = 1.0f;
   viTri.x = -130.0f;  viTri.y = 5.8f; viTri.z = 350.0f;
   danhSachVat[14].hinhDang.hinhCau = datHinhCau( 0.28f, &(danhSachVat[14].baoBiVT) );
   danhSachVat[14].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[14].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[14]), &phongTo, &xoay, &viTri );
   danhSachVat[14].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[14].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[14].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.8f;    mauTam.x = 1.0f;
   viTri.x = -131.0f;  viTri.y = 5.0f; viTri.z = 350.5f;
   danhSachVat[15].hinhDang.hinhCau = datHinhCau( 0.30f, &(danhSachVat[15].baoBiVT) );
   danhSachVat[15].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[15].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[15]), &phongTo, &xoay, &viTri );
   danhSachVat[15].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[15].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[15].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.8f;    mauTam.x = 1.0f;
   viTri.x = -143.0f;  viTri.y = 10.0f; viTri.z = 350.5f;
   danhSachVat[16].hinhDang.hinhCau = datHinhCau( 0.25f, &(danhSachVat[16].baoBiVT) );
   danhSachVat[16].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[16].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[16]), &phongTo, &xoay, &viTri );
   danhSachVat[16].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[16].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[16].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.8f;    mauTam.x = 1.0f;
   viTri.x = -158.0f;  viTri.y = 18.0f; viTri.z = 351.0f;
   danhSachVat[17].hinhDang.hinhCau = datHinhCau( 0.3f, &(danhSachVat[17].baoBiVT) );
   danhSachVat[17].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[17].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[17]), &phongTo, &xoay, &viTri );
   danhSachVat[17].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[17].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[17].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.8f;    mauTam.x = 1.0f;
   viTri.x = -157.0f;  viTri.y = 7.0f; viTri.z = 355.0f;
   danhSachVat[18].hinhDang.hinhCau = datHinhCau( 0.25f, &(danhSachVat[18].baoBiVT) );
   danhSachVat[18].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[18].chietSuat = 11.0f;
   datBienHoaChoVat( &(danhSachVat[18]), &phongTo, &xoay, &viTri );
   danhSachVat[18].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[18].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[18].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.8f;    mauTam.x = 1.0f;
   viTri.x = -153.0f;  viTri.y = 10.5f; viTri.z = 360.0f;
   danhSachVat[19].hinhDang.hinhCau = datHinhCau( 0.2f, &(danhSachVat[19].baoBiVT) );
   danhSachVat[19].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[19].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[19]), &phongTo, &xoay, &viTri );
   danhSachVat[19].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[19].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[19].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.8f;    mauTam.x = 1.0f;
   viTri.x = -155.0f;  viTri.y = 15.5f; viTri.z = 306.0f;
   danhSachVat[20].hinhDang.hinhCau = datHinhCau( 0.1f, &(danhSachVat[20].baoBiVT) );
   danhSachVat[20].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[20].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[20]), &phongTo, &xoay, &viTri );
   danhSachVat[20].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[20].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[20].viTriDau = viTri;
   
   // ---- nhóm
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.8f;    mauTam.x = 1.0f;
   viTri.x = -163.0f;  viTri.y = 13.8f; viTri.z = 275.0f;
   danhSachVat[21].hinhDang.hinhCau = datHinhCau( 0.22f, &(danhSachVat[21].baoBiVT) );
   danhSachVat[21].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[21].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[21]), &phongTo, &xoay, &viTri );
   danhSachVat[21].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[21].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[21].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.8f;    mauTam.x = 1.0f;
   viTri.x = -162.0f;  viTri.y = 14.3f; viTri.z = 280.0f;
   danhSachVat[22].hinhDang.hinhCau = datHinhCau( 0.1f, &(danhSachVat[22].baoBiVT) );
   danhSachVat[22].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[22].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[22]), &phongTo, &xoay, &viTri );
   danhSachVat[22].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[22].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[22].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.5f;    mauTam.x = 0.5f;
   viTri.x = -152.0f;  viTri.y = 14.3f; viTri.z = 278.0f;
   danhSachVat[23].hinhDang.hinhCau = datHinhCau( 0.18f, &(danhSachVat[23].baoBiVT) );
   danhSachVat[23].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[23].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[23]), &phongTo, &xoay, &viTri );
   danhSachVat[23].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[23].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[23].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 0.5f;
   viTri.x = -172.0f;  viTri.y = 17.8f; viTri.z = 275.0f;
   danhSachVat[24].hinhDang.hinhCau = datHinhCau( 0.19f, &(danhSachVat[24].baoBiVT) );
   danhSachVat[24].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[24].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[24]), &phongTo, &xoay, &viTri );
   danhSachVat[24].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[24].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[24].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 0.5f;
   viTri.x = -180.0f;  viTri.y = 15.6f; viTri.z = 270.0f;
   danhSachVat[25].hinhDang.hinhCau = datHinhCau( 0.21f, &(danhSachVat[25].baoBiVT) );
   danhSachVat[25].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[25].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[25]), &phongTo, &xoay, &viTri );
   danhSachVat[25].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[25].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[25].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -130.0f;  viTri.y = 14.6f; viTri.z = 272.0f;
   danhSachVat[26].hinhDang.hinhCau = datHinhCau( 0.35f, &(danhSachVat[26].baoBiVT) );
   danhSachVat[26].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[26].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[26]), &phongTo, &xoay, &viTri );
   danhSachVat[26].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[26].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[26].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -134.0f;  viTri.y = 18.0f; viTri.z = 278.0f;
   danhSachVat[27].hinhDang.hinhCau = datHinhCau( 0.35f, &(danhSachVat[27].baoBiVT) );
   danhSachVat[27].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[27].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[27]), &phongTo, &xoay, &viTri );
   danhSachVat[27].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[27].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[27].viTriDau = viTri;

   // ---- độc lập
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 0.5f;
   viTri.x = -131.0f;  viTri.y = 13.9f; viTri.z = 230.0f;
   danhSachVat[28].hinhDang.hinhCau = datHinhCau( 0.14f, &(danhSachVat[28].baoBiVT) );
   danhSachVat[28].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[28].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[28]), &phongTo, &xoay, &viTri );
   danhSachVat[28].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[28].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[28].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 0.5f;
   viTri.x = -135.0f;  viTri.y = 17.2f; viTri.z = 218.0f;
   danhSachVat[29].hinhDang.hinhCau = datHinhCau( 0.3f, &(danhSachVat[29].baoBiVT) );
   danhSachVat[29].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[29].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[29]), &phongTo, &xoay, &viTri );
   danhSachVat[29].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[29].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[29].viTriDau = viTri;
   
   // ---- nhóm
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 0.5f;
   viTri.x = -140.0f;  viTri.y = 16.0f; viTri.z = 288.0f;
   danhSachVat[30].hinhDang.hinhCau = datHinhCau( 0.13f, &(danhSachVat[30].baoBiVT) );
   danhSachVat[30].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[30].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[30]), &phongTo, &xoay, &viTri );
   danhSachVat[30].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[30].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[30].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.7f;    mauRanh.x = 0.0f;
   mauTam.d = 1.0f;    mauTam.l = 1.0f;    mauTam.x = 0.5f;
   viTri.x = -143.0f;  viTri.y = 5.5f; viTri.z = 285.0f;
   danhSachVat[31].hinhDang.hinhCau = datHinhCau( 0.3f, &(danhSachVat[31].baoBiVT) );
   danhSachVat[31].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[31].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[31]), &phongTo, &xoay, &viTri );
   danhSachVat[31].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[31].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[31].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -143.0f;  viTri.y = 6.5f; viTri.z = 275.0f;
   danhSachVat[32].hinhDang.hinhCau = datHinhCau( 0.25f, &(danhSachVat[32].baoBiVT) );
   danhSachVat[32].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[32].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[32]), &phongTo, &xoay, &viTri );
   danhSachVat[32].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[32].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[32].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -142.0f;  viTri.y = 6.5f; viTri.z = 284.0f;
   danhSachVat[33].hinhDang.hinhCau = datHinhCau( 0.25f, &(danhSachVat[33].baoBiVT) );
   danhSachVat[33].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[33].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[33]), &phongTo, &xoay, &viTri );
   danhSachVat[33].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[33].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[33].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -130.0f;  viTri.y = 16.5f; viTri.z = 275.0f;
   danhSachVat[34].hinhDang.hinhCau = datHinhCau( 0.1f, &(danhSachVat[34].baoBiVT) );
   danhSachVat[34].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[34].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[34]), &phongTo, &xoay, &viTri );
   danhSachVat[34].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[34].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[34].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -140.0f;  viTri.y = 15.3f; viTri.z = 285.0f;
   danhSachVat[35].hinhDang.hinhCau = datHinhCau( 0.17f, &(danhSachVat[35].baoBiVT) );
   danhSachVat[35].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[35].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[35]), &phongTo, &xoay, &viTri );
   danhSachVat[35].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[35].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[35].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.6f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -148.0f;  viTri.y = 14.0f; viTri.z = 273.0f;
   danhSachVat[36].hinhDang.hinhCau = datHinhCau( 0.13f, &(danhSachVat[36].baoBiVT) );
   danhSachVat[36].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[36].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[36]), &phongTo, &xoay, &viTri );
   danhSachVat[36].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[36].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[36].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.6f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 12.0f;    mauTam.x = 1.0f;
   viTri.x = -158.0f;  viTri.y = 3.8f; viTri.z = 263.0f;
   danhSachVat[37].hinhDang.hinhCau = datHinhCau( 0.3f, &(danhSachVat[37].baoBiVT) );
   danhSachVat[37].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[37].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[37]), &phongTo, &xoay, &viTri );
   danhSachVat[37].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[37].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[37].viTriDau = viTri;
   
   // ----
   mauRanh.d = 0.0f;    mauRanh.l = 0.9f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -155.0f;  viTri.y = 4.8f; viTri.z = 315.0f;
   danhSachVat[38].hinhDang.hinhCau = datHinhCau( 0.35f, &(danhSachVat[38].baoBiVT) );
   danhSachVat[38].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[38].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[38]), &phongTo, &xoay, &viTri );
   danhSachVat[38].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[38].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[38].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.1f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 0.5f;
   viTri.x = -130.0f;  viTri.y = 4.0f; viTri.z = 300.0f;
   danhSachVat[39].hinhDang.hinhCau = datHinhCau( 0.19f, &(danhSachVat[39].baoBiVT) );
   danhSachVat[39].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[39].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[39]), &phongTo, &xoay, &viTri );
   danhSachVat[39].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[39].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[39].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -142.0f;  viTri.y = 3.6f; viTri.z = 302.0f;
   danhSachVat[40].hinhDang.hinhCau = datHinhCau( 0.4f, &(danhSachVat[40].baoBiVT) );
   danhSachVat[40].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[40].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[40]), &phongTo, &xoay, &viTri );
   danhSachVat[40].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[40].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[40].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 1.0f;    mauRanh.x = 0.0f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -132.0f;  viTri.y = 5.6f; viTri.z = 300.0f;
   danhSachVat[41].hinhDang.hinhCau = datHinhCau( 0.4f, &(danhSachVat[41].baoBiVT) );
   danhSachVat[41].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[41].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[41]), &phongTo, &xoay, &viTri );
   danhSachVat[41].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[41].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[41].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.5f;    mauTam.l = 1.0f;    mauTam.x = 0.0f;
   viTri.x = -136.0f;  viTri.y = 6.5f; viTri.z = 280.0f;
   danhSachVat[42].hinhDang.hinhCau = datHinhCau( 0.2f, &(danhSachVat[42].baoBiVT) );
   danhSachVat[42].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[42].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[42]), &phongTo, &xoay, &viTri );
   danhSachVat[42].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[42].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[42].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.1f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 1.0f;    mauTam.x = 0.0f;
   viTri.x = -150.0f;  viTri.y = 7.0f; viTri.z = 295.0f;
   danhSachVat[43].hinhDang.hinhCau = datHinhCau( 0.1f, &(danhSachVat[43].baoBiVT) );
   danhSachVat[43].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[43].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[43]), &phongTo, &xoay, &viTri );
   danhSachVat[43].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[43].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[43].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.7f;    mauRanh.x = 1.0f;
   mauTam.d = 1.0f;    mauTam.l = 1.0f;    mauTam.x = 0.0f;
   viTri.x = -188.0f;  viTri.y = 4.2f; viTri.z = 264.0f;
   danhSachVat[44].hinhDang.hinhCau = datHinhCau( 0.1f, &(danhSachVat[44].baoBiVT) );
   danhSachVat[44].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[44].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[44]), &phongTo, &xoay, &viTri );
   danhSachVat[44].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[44].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[44].viTriDau = viTri;

   mauRanh.d = 0.8f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -193.0f;  viTri.y = 8.0f; viTri.z = 293.0f;
   danhSachVat[45].hinhDang.hinhCau = datHinhCau( 0.22f, &(danhSachVat[45].baoBiVT) );
   danhSachVat[45].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[45].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[45]), &phongTo, &xoay, &viTri );
   danhSachVat[45].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[45].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[45].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.2f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 0.0f;
   viTri.x = -150.0f;  viTri.y = 14.6f; viTri.z = 283.0f;
   danhSachVat[46].hinhDang.hinhCau = datHinhCau( 0.3f, &(danhSachVat[46].baoBiVT) );
   danhSachVat[46].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[46].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[46]), &phongTo, &xoay, &viTri );
   danhSachVat[46].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[46].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[46].viTriDau = viTri;

   
   mauRanh.d = 0.0f;    mauRanh.l = 0.7f;    mauRanh.x = 0.7f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -155.0f;  viTri.y = 10.0f; viTri.z = 308.0f;
   danhSachVat[47].hinhDang.hinhCau = datHinhCau( 0.1f, &(danhSachVat[47].baoBiVT) );
   danhSachVat[47].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[47].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[47]), &phongTo, &xoay, &viTri );
   danhSachVat[47].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[47].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[47].viTriDau = viTri;

   mauRanh.d = 0.4f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -141.0f;  viTri.y = 24.0f; viTri.z = 284.0f;
   danhSachVat[48].hinhDang.hinhCau = datHinhCau( 0.2f, &(danhSachVat[48].baoBiVT) );
   danhSachVat[48].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[48].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[48]), &phongTo, &xoay, &viTri );
   danhSachVat[48].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[48].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[48].viTriDau = viTri;

   // ---- nhóm bong bóng cuối
   mauRanh.d = 0.4f;    mauRanh.l = 0.0f;    mauRanh.x = 1.0f;
   mauTam.d = 0.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -811.0f;  viTri.y = 25.0f; viTri.z = 1132.0f + 30.0f;
   danhSachVat[49].hinhDang.hinhCau = datHinhCau( 0.25f, &(danhSachVat[49].baoBiVT) );
   danhSachVat[49].loai = kLOAI_HINH_DANG__HINH_CAU;

   danhSachVat[49].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[49]), &phongTo, &xoay, &viTri );
   danhSachVat[49].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[49].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[49].viTriDau = viTri;

   mauRanh.d = 0.0f;    mauRanh.l = 0.7f;    mauRanh.x = 0.7f;    mauRanh.dd = 0.3f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;    mauTam.dd = 0.0f;
   viTri.x = -807.0f;  viTri.y = 27.0f; viTri.z = 1135.0f + 30.0f;
   danhSachVat[50].hinhDang.hinhCau = datHinhCau( 0.3f, &(danhSachVat[50].baoBiVT) );
   danhSachVat[50].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[50].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[50]), &phongTo, &xoay, &viTri );
   danhSachVat[50].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[50].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[50].viTriDau = viTri;


   mauRanh.d = 0.0f;    mauRanh.l = 1.0f;    mauRanh.x = 1.0f;    mauRanh.dd = 0.2f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;    mauTam.dd = 0.0f;
   viTri.x = -838.0f;  viTri.y = 31.0f; viTri.z = 1184.0f + 30.0f;
   danhSachVat[51].hinhDang.hinhCau = datHinhCau( 0.25f, &(danhSachVat[51].baoBiVT) );
   danhSachVat[51].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[51].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[51]), &phongTo, &xoay, &viTri );
   danhSachVat[51].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[51].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[51].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -840.0f;  viTri.y = 32.0f; viTri.z = 1175.0f + 30.0f;
   danhSachVat[52].hinhDang.hinhCau = datHinhCau( 0.4f, &(danhSachVat[52].baoBiVT) );
   danhSachVat[52].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[52].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[52]), &phongTo, &xoay, &viTri );
   danhSachVat[52].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[52].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[52].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 0.7f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -870.0f;  viTri.y = 30.0f; viTri.z = 1240.0f + 30.0f;
   danhSachVat[53].hinhDang.hinhCau = datHinhCau( 0.2f, &(danhSachVat[53].baoBiVT) );
   danhSachVat[53].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[53].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[53]), &phongTo, &xoay, &viTri );
   danhSachVat[53].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[53].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[53].viTriDau = viTri;

   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 0.0f;
   mauTam.d = 0.0f;    mauTam.l = 1.0f;    mauTam.x = 1.0f;
   viTri.x = -873.0f;  viTri.y = 31.0f; viTri.z = 1235.0f + 30.0f;
   danhSachVat[54].hinhDang.hinhCau = datHinhCau( 0.5f, &(danhSachVat[54].baoBiVT) );
   danhSachVat[54].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[54].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[54]), &phongTo, &xoay, &viTri );
   danhSachVat[54].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[54].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[54].viTriDau = viTri;
   
   mauRanh.d = 1.0f;    mauRanh.l = 0.0f;    mauRanh.x = 0.7f;
   mauTam.d = 1.0f;    mauTam.l = 0.0f;    mauTam.x = 1.0f;
   viTri.x = -877.0f;  viTri.y = 34.0f; viTri.z = 1238.0f + 30.0f;
   danhSachVat[55].hinhDang.hinhCau = datHinhCau( 0.35f, &(danhSachVat[55].baoBiVT) );
   danhSachVat[55].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[55].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[55]), &phongTo, &xoay, &viTri );
   danhSachVat[55].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
   danhSachVat[55].soHoaTiet = kHOA_TIET__DI_HUONG;
   danhSachVat[55].viTriDau = viTri;

   return 56;
}

unsigned short datCacBatDienXoay( VatThe *danhSachVat ) {
   
   Vecto truc;
   truc.x = 1.0f;    truc.y = 1.0f;    truc.z = 0.0f;
   Quaternion xoay;
   xoay.w = 1.0f;   xoay.x = 0.0f;    xoay.y = 0.0f;    xoay.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   Mau mau;
   mau.d = 1.0f;    mau.l = 1.0f;    mau.x = 1.0f;    mau.dd = 0.1f;     mau.p = 0.5f;


   // ---- hai bát diện tại -x
   viTri.x = -15.0f;  viTri.y = 17.0f; viTri.z = 15.0f;
   danhSachVat[0].hinhDang.batDien = datBatDien( 4.0f, 4.5f, 4.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[0].chietSuat = 2.4f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &xoay, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[0].viTriDau = viTri;
   
   viTri.y = 22.0f;
   danhSachVat[1].hinhDang.batDien = datBatDien( 2.2f, 2.5f, 2.2f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[1].chietSuat = 2.4f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &xoay, &viTri );
   danhSachVat[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[1].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[1].viTriDau = viTri;
   
   viTri.y = 26.0f;
   danhSachVat[2].hinhDang.batDien = datBatDien( 1.5f, 1.7f, 1.5f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[2].chietSuat = 2.4f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &xoay, &viTri );
   danhSachVat[2].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[2].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[2].viTriDau = viTri;
   
   // ---- hai bát diện tại -z
   viTri.x = 15.0f;  viTri.y = 17.0f; viTri.z = -15.0f;
   danhSachVat[3].hinhDang.batDien = datBatDien( 4.0f, 4.5f, 4.0f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[3].chietSuat = 2.4f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &xoay, &viTri );
   danhSachVat[3].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[3].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[3].viTriDau = viTri;
   
   viTri.y = 22.0f;
   danhSachVat[4].hinhDang.batDien = datBatDien( 2.2f, 2.5f, 2.2f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[4].chietSuat = 2.4f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &xoay, &viTri );
   danhSachVat[4].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[4].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[4].viTriDau = viTri;

   viTri.y = 26.0f;
   danhSachVat[5].hinhDang.batDien = datBatDien( 1.5f, 1.7f, 1.5f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[5].chietSuat = 2.4f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &xoay, &viTri );
   danhSachVat[5].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[5].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[5].viTriDau = viTri;
   
   return 6;
}

unsigned short datCotNhay( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;    quaternion.x = 0.0f;     quaternion.y = 0.0f;     quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   mau.d = 0.5f;    mau.l = 0.3f;    mau.x = 0.6f;    mau.dd = 1.0f;    mau.p = 0.5f;
   Vecto viTri;
   
   float goc = 3.1415926f*0.8f;
   
   unsigned short soVatThe = 0;
   unsigned char soCot = 0;
   while ( soCot < 12 ) {
      viTri.x = 17.5f*cosf( goc );       viTri.y = 11.0f;       viTri.z = 17.5f*sinf( goc );

      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 0.7f, 0.8f, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soVatThe].viTriDau = viTri;
      soVatThe++;
      
      viTri.y += 0.5f;
      danhSachVat[soVatThe].hinhDang.hinhTru = datHinhTru( 0.8f, 0.9f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soVatThe].viTriDau = viTri;
      soVatThe++;
      
      viTri.y += 0.5f;
      danhSachVat[soVatThe].hinhDang.hinhNon = datHinhNon( 0.8f, 0.7f, 0.1f, &(danhSachVat[soVatThe].baoBiVT) );
      danhSachVat[soVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soVatThe].viTriDau = viTri;
      soVatThe++;
      
      soCot++;
      goc += -3.14159f*0.1f;
   }
   
   return soVatThe;
}


#pragma mark ---- NÂNG CẤP PHIM TRƯỜNG 0
void nangCapPhimTruong0( PhimTruong *phimTruong ) {

   nangCapPhimTruong0_mayQuayPhim( phimTruong );
   nangCapPhimTruong0_nhanVat( phimTruong, phimTruong->soHoatHinhDau );
   
   // ---- tăng số hoạt hình
   phimTruong->soHoatHinhDau++;
}


void nangCapPhimTruong0_mayQuayPhim( PhimTruong *phimTruong ) {
   
   unsigned short soHoatHinh = phimTruong->soHoatHinhDau;
   MayQuayPhim *mayQuayPhim = &(phimTruong->mayQuayPhim);
 

    // ---- cho xem từ trời xuống
/*    mayQuayPhim->viTri.x = -150.0f;//800.0f - 2.0f*soHoatHinh;
    mayQuayPhim->viTri.y = 150.0f;
    mayQuayPhim->viTri.z = -90.0f;
   
    Vecto huongNhin;
    huongNhin.x = -0.01f;
    huongNhin.y = -1.0f;
    huongNhin.z = -0.0f;  // 1,5 là vận tốc trái banh vai chánh và sao gai
    dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   // ====== Xài này cho xem các trái banh đi quanh vòng xuyến
   mayQuayPhim->viTri.x = 30.0f;
   mayQuayPhim->viTri.y = 80.0f;
   mayQuayPhim->viTri.z = 30.0f;
   Vecto huongNhin;
   huongNhin.x = 0.0f - mayQuayPhim->viTri.x;
   huongNhin.y = 10.0f - mayQuayPhim->viTri.y;
   huongNhin.z = 0.0f - mayQuayPhim->viTri.z;
   dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin ); */

 
   /* // ====== Xài này cho xem trái banh rớt
    mayQuayPhim->viTri.x = -30.0f;
    mayQuayPhim->viTri.y = 40.0f;
    mayQuayPhim->viTri.z = -62.0f;
    Vecto huongNhin;
    huongNhin.x = -30.0f - mayQuayPhim->viTri.x;
    huongNhin.y = 13.0f - mayQuayPhim->viTri.y;
    huongNhin.z = -50.0f - mayQuayPhim->viTri.z;
    dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
    */

   // ====== Xài này cho xem sao gai rớt
/*    mayQuayPhim->viTri.x = -50.0f;
    mayQuayPhim->viTri.y = 20.0f;
    mayQuayPhim->viTri.z = -82.0f;
    Vecto huongNhin;
    huongNhin.x = -45.0f - mayQuayPhim->viTri.x;
    huongNhin.y = 20.0f - mayQuayPhim->viTri.y;
    huongNhin.z = -40.0f - mayQuayPhim->viTri.z;
    dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin ); */


   // ---- cho 240 hoạt hình đầu di chuyển máy quay phim
   if( soHoatHinh < 240 ) { // 10 giây - đi thẩn đến bờ
      
      float buoc = -400.0f*soHoatHinh/240.0f;
 
      mayQuayPhim->viTri.x = 0.0f;
      mayQuayPhim->viTri.y = 3.0f;
      mayQuayPhim->viTri.z = 500.0 + buoc;
      //      printf( "mayQuayPhim->viTri.z %5.3f\n", mayQuayPhim->viTri.z );
   }
   else if( soHoatHinh < 368 ) {  //  giây - bặt đầu bay lên
      Bezier bezier;
      bezier.diemQuanTri[0].x = 0.0f;
      bezier.diemQuanTri[0].y = 3.0f;
      bezier.diemQuanTri[0].z = 100.0f;
      bezier.diemQuanTri[1].x = 0.0f;
      bezier.diemQuanTri[1].y = 3.0f;
      bezier.diemQuanTri[1].z = 70.0f;
      
      bezier.diemQuanTri[2].x = 13.0f;
      bezier.diemQuanTri[2].y = 22.0f;
      bezier.diemQuanTri[2].z = 37.0f;
      
      bezier.diemQuanTri[3].x = 27.0f;
      bezier.diemQuanTri[3].y = 25.0f;
      bezier.diemQuanTri[3].z = 17.0f;
      float buoc = (soHoatHinh-240)/128.0f;
      // ---- -(1 - x)^3 + 1
      buoc = (1.0f - buoc);
      buoc = -buoc*buoc*buoc + 1;
      
      Vecto viTri = tinhViTriBezier3C( &bezier, buoc );
      mayQuayPhim->viTri.x = viTri.x;
      mayQuayPhim->viTri.y = viTri.y;
      mayQuayPhim->viTri.z = viTri.z;
      
      float buocY = buoc*(10.0f - 3.0f);
      Vecto huongNhin;
      huongNhin.x = 0.0f - viTri.x;
      huongNhin.y = buocY + 3.0f - viTri.y;
      huongNhin.z = 0.0f - viTri.z;
      
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 440 ) {  // ngó vòng xuyến
      mayQuayPhim->viTri.x = 27.0f;
      mayQuayPhim->viTri.y = 25.0f;
      mayQuayPhim->viTri.z = 17.0f;
      Vecto huongNhin;
      huongNhin.x = 0.0f - mayQuayPhim->viTri.x;
      huongNhin.y = 10.0f - mayQuayPhim->viTri.y;
      huongNhin.z = 0.0f - mayQuayPhim->viTri.z;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );

   }
   else if( soHoatHinh < 550 ) {  // ngó vòng xuyến
      Bezier bezier;
      bezier.diemQuanTri[0].x = 27.0f;
      bezier.diemQuanTri[0].y = 25.0f;
      bezier.diemQuanTri[0].z = 17.0f;
      
      bezier.diemQuanTri[1].x = 22.5f;
      bezier.diemQuanTri[1].y = 28.9f;
      bezier.diemQuanTri[1].z =  6.4f;
      
      bezier.diemQuanTri[2].x = 11.0f;
      bezier.diemQuanTri[2].y = 38.0f;
      bezier.diemQuanTri[2].z = 3.0f;
      
      bezier.diemQuanTri[3].x =  9.0f;
      bezier.diemQuanTri[3].y = 38.0f;
      bezier.diemQuanTri[3].z =  3.0f;

      // cho di chuyển mịn
      float chuKy = 550.0f - 440.0f;
      float buoc = -0.5f*cosf( (soHoatHinh-440)*3.14159/chuKy ) + 0.5f;
      
      Vecto viTri = tinhViTriBezier3C( &bezier, buoc );
      mayQuayPhim->viTri.x = viTri.x;
      mayQuayPhim->viTri.y = viTri.y;
      mayQuayPhim->viTri.z = viTri.z;

      float buocNhin = -11.0f/(550.0f - 440.0f);
      Vecto huongNhin;

      huongNhin.x = buocNhin*(soHoatHinh-440) - mayQuayPhim->viTri.x;
      huongNhin.y = 10.0f - mayQuayPhim->viTri.y;
      huongNhin.z = 0.0f - mayQuayPhim->viTri.z;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
//      printf();
   }
   else if( soHoatHinh < 600 ) {   // ngó vai chánh
      mayQuayPhim->viTri.x = 9.0f;
      mayQuayPhim->viTri.y = 38.0f;
      mayQuayPhim->viTri.z = 3.0f;

      Vecto huongNhin;
      
      huongNhin.x = -11.0f - mayQuayPhim->viTri.x;
      huongNhin.y = 10.0f - mayQuayPhim->viTri.y;
      huongNhin.z = 0.0f - mayQuayPhim->viTri.z;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 650 ) {
      
      // cho di chuyển mịn
      float chuKy = 650.0f - 600.0f;

      mayQuayPhim->viTri.x = 9.0;//viTri.x;
      mayQuayPhim->viTri.y = 38.0f;//viTri.y;
      mayQuayPhim->viTri.z = 3.0f;//viTri.z;
      
      float gocNhin = (soHoatHinh-600)*0.5f*3.1415926/chuKy; // 100 = 650 - 550
//      printf( "gocNhin %5.3f\n", gocNhin );
      Vecto huongNhin;
      huongNhin.x = -11.0f*cosf( gocNhin ) - mayQuayPhim->viTri.x;
      huongNhin.y = 10.0f - mayQuayPhim->viTri.y;
      huongNhin.z = -11.0f*sinf( gocNhin ) - mayQuayPhim->viTri.z;
//      printf( "hướngNhìn %5.3f %5.3f %5.3f\n", huongNhin.x, huongNhin.y, huongNhin.z );
      //      printf( "mayQuayPhim %5.3f %5.3f %5.3f\n", mayQuayPhim->viTri.x, mayQuayPhim->viTri.y, mayQuayPhim->viTri.z );
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 660 ) { // xem trái banh bay ra ngoài
      mayQuayPhim->viTri.x = 9.0;//viTri.x;
      mayQuayPhim->viTri.y = 38.0f;//viTri.y;
      mayQuayPhim->viTri.z = 3.0f;//viTri.z;
      
      Vecto huongNhin;
      huongNhin.x = 0.0f - mayQuayPhim->viTri.x;
      huongNhin.y = 10.0f - mayQuayPhim->viTri.y;
      huongNhin.z = -11.0 - mayQuayPhim->viTri.z;

      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 675 ) {   // trá banh trúng kiến lồng sao gai
      mayQuayPhim->viTri.x = -30.0f;
      mayQuayPhim->viTri.y = 23.0f;
      mayQuayPhim->viTri.z = -52.0f;
      
      Vecto huongNhin;
      huongNhin.x = -1.0f;
      huongNhin.y =  0.2f;
      huongNhin.z =  0.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 690 ) {  // nhìn từ trời xuống, trái banh nhồi lại
      mayQuayPhim->viTri.x = -40.0f;
      mayQuayPhim->viTri.y = 50.0f;
      mayQuayPhim->viTri.z = -40.0f;
   
      Vecto huongNhin;
      huongNhin.x = -52.0f - mayQuayPhim->viTri.x;
      huongNhin.y = 8.0f - mayQuayPhim->viTri.y;
      huongNhin.z = -52.0f - mayQuayPhim->viTri.z;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 715 ) {  // nhìn từ trời xuống
      mayQuayPhim->viTri.x = -28.0f;
      mayQuayPhim->viTri.y = 20.0f;
      mayQuayPhim->viTri.z = -54.0f;

      Vecto huongNhin;
      huongNhin.x = -1.0f;
      huongNhin.y = -0.3f;
      huongNhin.z = 1.0f ;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 780 ) {  // sao gai chạy thoát
      mayQuayPhim->viTri.x = -30.0f;
      mayQuayPhim->viTri.y = 23.0f;
      mayQuayPhim->viTri.z = -52.0f;
      
      Vecto huongNhin;
      huongNhin.x = -1.0f;
      huongNhin.y =  0.2f;
      huongNhin.z =  0.0f;
      
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 820 ) {
      Bezier bezier;
      bezier.diemQuanTri[0].x = -30.0f;
      bezier.diemQuanTri[0].y = 23.0f;
      bezier.diemQuanTri[0].z = -52.0f;
      
      bezier.diemQuanTri[1].x = -30.0f;
      bezier.diemQuanTri[1].y = 23.0f;
      bezier.diemQuanTri[1].z = -52.4f;
      
      bezier.diemQuanTri[2].x = -15.0f;
      bezier.diemQuanTri[2].y = 23.0f;
      bezier.diemQuanTri[2].z = -60.0f;
      
      bezier.diemQuanTri[3].x = -14.0f;
      bezier.diemQuanTri[3].y = 23.0f;
      bezier.diemQuanTri[3].z = -70.0f;

      unsigned short soHoatHinhTuongDoi = soHoatHinh - 780;
      Vecto viTri = tinhViTriBezier3C( &bezier, 0.025*soHoatHinhTuongDoi );  // 0.05 = 1/20
      
      mayQuayPhim->viTri.x = viTri.x;
      mayQuayPhim->viTri.y = viTri.y;
      mayQuayPhim->viTri.z = viTri.z;
   
      Vecto huongNhin;
      huongNhin.x = -1.0f*cosf( soHoatHinhTuongDoi*0.02f );
      huongNhin.y =  0.2f - 0.015f*soHoatHinhTuongDoi;
      huongNhin.z =  1.0f*sinf( soHoatHinhTuongDoi*0.02f );
      
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 875 ) {
      mayQuayPhim->viTri.x = -14.0f;
      mayQuayPhim->viTri.y = 23.0f;
      mayQuayPhim->viTri.z = -70.0f;

      unsigned short soHoatHinhTuongDoi = 820 - 780;
      Vecto huongNhin;
      huongNhin.x = -1.0f*cosf( soHoatHinhTuongDoi*0.02f );
      huongNhin.y =  0.2f - 0.015f*soHoatHinhTuongDoi;
      huongNhin.z =  1.0f*sinf( soHoatHinhTuongDoi*0.02f );
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 910 ) {
       mayQuayPhim->viTri.x = -48.0f;
       mayQuayPhim->viTri.y = 19.0f;
       mayQuayPhim->viTri.z = -54.0f;
       Vecto huongNhin;
       huongNhin.x = 30.0f - mayQuayPhim->viTri.x;
       huongNhin.y = 5.0f - mayQuayPhim->viTri.y;
       huongNhin.z = -50.0f - mayQuayPhim->viTri.z;
       dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
    }
   else if( soHoatHinh < 1025 ) {
      mayQuayPhim->viTri.x = -10.0f;
      mayQuayPhim->viTri.y = 35.0f;
      mayQuayPhim->viTri.z = -41.0f;
      Vecto huongNhin;
      huongNhin.x = 0.0f;
      huongNhin.y = -1.0f;
      huongNhin.z = -0.4f;

      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1055 ) {
      unsigned short soHoatHinhTuongDoi = soHoatHinh - 1025;
      mayQuayPhim->viTri.x = -3.0f;
      mayQuayPhim->viTri.y = 13.0f + 0.1f*soHoatHinhTuongDoi;
      mayQuayPhim->viTri.z = -50.0f + 0.1f*soHoatHinhTuongDoi;
      Vecto huongNhin;
      huongNhin.x = 0.0f;
      huongNhin.y = -0.1f;
      huongNhin.z = 1.0f;
      
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1170 ) {
      float goc = 0.5f + 0.04f*(soHoatHinh - 1055);
      mayQuayPhim->viTri.x = -35.0f*sinf( goc );
      mayQuayPhim->viTri.y = 30.0f;
      mayQuayPhim->viTri.z = -35.0f*cosf( goc );
      Vecto huongNhin;
      huongNhin.x = 0.0f - mayQuayPhim->viTri.x;
      huongNhin.y = 10.0f - mayQuayPhim->viTri.y;
      huongNhin.z = 0.0f - mayQuayPhim->viTri.z;
      
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1280 ) {
      unsigned short soHoatHinhTuongDoi = soHoatHinh - 1170;
      mayQuayPhim->viTri.x = 3.0f;
      mayQuayPhim->viTri.y = 20.0f;
      mayQuayPhim->viTri.z = -13.0f - 1.15f*soHoatHinhTuongDoi;
      Vecto huongNhin;
      huongNhin.x = 0.0f;
      huongNhin.y = -20.0f;
      huongNhin.z = -25.0f - soHoatHinhTuongDoi*0.4f;  // theo trái bánh và sao gai
      printf( "%d  mayQuayPhim->huongNhin %5.3f %5.3f %5.3f\n", soHoatHinh, huongNhin.x, huongNhin.y, huongNhin.z );
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1335 ) { // giảm tốc độ và bay lên một chút
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;   congBezier.diemQuanTri[0].y = 20.0f;   congBezier.diemQuanTri[0].z = -139.5f;
      congBezier.diemQuanTri[1].x = 3.0f;   congBezier.diemQuanTri[1].y = 20.0f;   congBezier.diemQuanTri[1].z = -162.0f;
      congBezier.diemQuanTri[2].x = 3.0f;   congBezier.diemQuanTri[2].y = 22.4f;   congBezier.diemQuanTri[2].z = -169.3f;
      congBezier.diemQuanTri[3].x = 3.0f;   congBezier.diemQuanTri[3].y = 24.7f;   congBezier.diemQuanTri[3].z = -172.0f;

      float thamSoBezier = ((float)soHoatHinh - 1280.0f)/55.0f;
      Vecto viTri = tinhViTriBezier3C( &congBezier, thamSoBezier );
   
      mayQuayPhim->viTri = viTri;

      unsigned short soHoatHinhTuongDoi = soHoatHinh - 1280;
      Vecto huongNhin;
      huongNhin.x = 0.0f;
      huongNhin.y = -20.0f;
      huongNhin.z = -69.0f - soHoatHinhTuongDoi*0.4f;
      printf( "%d  mayQuayPhim->huongNhin %5.3f %5.3f %5.3f\n", soHoatHinh, huongNhin.x, huongNhin.y, huongNhin.z );

      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
      
//      printf( "%d  mayQuayPhim->viTri %5.3f %5.3f %5.3f\n", soHoatHinh, mayQuayPhim->viTri.x, mayQuayPhim->viTri.y, mayQuayPhim->viTri.z );
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.y*vanToc.y + vanToc.z*vanToc.z )/55.0f;
      printf( "---> thamSoBezier %5.3f  tocDo mayQuayPhim %5.3f\n", thamSoBezier, tocDo );
   }
   else if( soHoatHinh < 1440 ) {  // nghỉ tới mà vẫn bay lên một chút
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;   congBezier.diemQuanTri[0].y = 24.7f;   congBezier.diemQuanTri[0].z = -172.0f;
      congBezier.diemQuanTri[1].x = 3.0f;   congBezier.diemQuanTri[1].y = 28.3f;   congBezier.diemQuanTri[1].z = -178.8f;
      congBezier.diemQuanTri[2].x = 3.0f;   congBezier.diemQuanTri[2].y = 30.0f;   congBezier.diemQuanTri[2].z = -180.0f;
      congBezier.diemQuanTri[3].x = 3.0f;   congBezier.diemQuanTri[3].y = 30.0f;   congBezier.diemQuanTri[3].z = -180.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 1335.0f)/105.0f;
      Vecto viTri = tinhViTriBezier3C( &congBezier, thamSoBezier );
      
      mayQuayPhim->viTri = viTri;

      unsigned short soHoatHinhTuongDoi = soHoatHinh - 1280;
      Vecto huongNhin;
      huongNhin.x = 0.0f;
      huongNhin.y = -20.0f;
      huongNhin.z = -69.0f - soHoatHinhTuongDoi*0.4f;
      printf( "%d  mayQuayPhim->huongNhin %5.3f %5.3f %5.3f\n", soHoatHinh, huongNhin.x, huongNhin.y, huongNhin.z );

      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
      
 //     printf( "%d  mayQuayPhim->viTri %5.3f %5.3f %5.3f\n", soHoatHinh, mayQuayPhim->viTri.x, mayQuayPhim->viTri.y, mayQuayPhim->viTri.z );
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.y*vanToc.y + vanToc.z*vanToc.z )/105.0f;
      printf( "---> thamSoBezier %5.3f  tocDo mayQuayPhim %5.3f\n", thamSoBezier, tocDo );
   }
   else if( soHoatHinh < 1600 ) {
      
      float goc = (soHoatHinh - 1280)/50.0f;
      
      mayQuayPhim->viTri.x = 3.0f;
      mayQuayPhim->viTri.y = 30.0f;
      mayQuayPhim->viTri.z = -180.0f;//-210.0f;
      Vecto huongNhin;
      huongNhin.x = 0.0f;
      huongNhin.y = -20.0f;
      huongNhin.z = -133.0f;  // -50 = -10 - 0.4*(1280-1180)
      printf( "%d  mayQuayPhim->huongNhin %5.3f %5.3f %5.3f\n", soHoatHinh, huongNhin.x, huongNhin.y, huongNhin.z );

      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   printf( "mayQuayPhim %5.3f %5.3f %5.3f\n", mayQuayPhim->viTri.x, mayQuayPhim->viTri.y, mayQuayPhim->viTri.z );
 
}

void nangCapPhimTruong0_nhanVat( PhimTruong *phimTruong, unsigned short soHoatHinh ) {

   // ---- nhân vật - các trai banh
   // ---- cho các trái banh né sao gai
   Vecto viTriSaoGai;
   viTriSaoGai.x = phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__SAO_GAI]].dich[12];
   viTriSaoGai.y = phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__SAO_GAI]].dich[13];
   viTriSaoGai.z = phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__SAO_GAI]].dich[14];
   moPhongTraiBanhGiaoThong( phimTruong->danhSachVatThe, phimTruong->nhanVat[kNHAN_VAT__BAY_TRAI_BANH_DAU],
                               phimTruong->nhanVat[kNHAN_VAT__BAY_TRAI_BANH_CUOI], &viTriSaoGai );

   // ---- nâng cấp các nhân vật
   nangCapVaiChanh( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__VAI_CHANH]]), soHoatHinh );
   nangCapSaoGai( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__SAO_GAI]]), soHoatHinh );
   nangCapTraiBanhBiGiet( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__TRAI_BANH_BI_GIET]]), soHoatHinh );
   nangCapKienLongSaoGai( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__KIEN_LONG]]), soHoatHinh );
   nangCapHatBayQuanhVongXuyen( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__HAT_THUY_TINH_BAY]]), soHoatHinh );
   nangCapBongBongBay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__BONG_BONG_DAU]]), phimTruong->nhanVat[kNHAN_VAT__BONG_BONG_CUOI] - phimTruong->nhanVat[kNHAN_VAT__BONG_BONG_DAU], soHoatHinh );
   nangCapCacBatDienXoay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__BAT_DIEN_XOAY_DAU]]), phimTruong->nhanVat[kNHAN_VAT__BAT_DIEN_XOAY_CUOI] - phimTruong->nhanVat[kNHAN_VAT__BAT_DIEN_XOAY_DAU], soHoatHinh );
   nangCapCotNhay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__COT_NHAY_DAU]]), phimTruong->nhanVat[kNHAN_VAT__COT_NHAY_CUOI] - phimTruong->nhanVat[kNHAN_VAT__COT_NHAY_DAU], soHoatHinh );
}

#define kVAI_CHANH__THAI_TRANG_CHAY_TANG_1      0
#define kVAI_CHANH__THAI_TRANG_NHAY_XUONG_1_0   1
#define kVAI_CHANH__THAI_TRANG_CHAY_TANG_0      2
#define kVAI_CHANH__THAI_TRANG_NHAY_XUONG_0_SAN 3
#define kVAI_CHANH__THAI_TRANG_CHAY_SAN         4
#define kVAI_CHANH__THAI_TRANG_NHAY_XUONG_SAN_DAT 5
#define kVAI_CHANH__THAI_TRANG_CHAY_DAT 6


void nangCapVaiChanh( VatThe *vaiChanh, unsigned short soHoatHinh ) {

   Vecto phongTo;
   phongTo.x = 1.0f;   phongTo.y = 1.0f;   phongTo.z = 1.0f;  // không bao giừ đổi, cần cho nâng các cấp biến hóa
   
   Quaternion xoay;
   xoay.w = 1.0f;    xoay.x = 0.0f;   xoay.y = 0.0f;   xoay.z = 0.0f;

   // ---- vị tri
   Vecto viTriVaiChanh;
   viTriVaiChanh.x = -10.0f;    viTriVaiChanh.y = 10.8f;    viTriVaiChanh.z = 10.0f;  // cần có ví trí sẵn, cho không có giá trị bậy

   // ---- vận tốc
   Vecto vanToc;
   float tocDo = 0.0f;
   // ---- trục xoay
   Vecto trucXoay;
   
   // ---- bán kính
   float banKinh = vaiChanh->hinhDang.hinhCau.banKinh;
   
   // ---- vectơ thẳnh lên (cho tính trục xoay)
   Vecto phapThuyenMatDat;
   phapThuyenMatDat.x = 0.0f;   phapThuyenMatDat.y = 1.0f;   phapThuyenMatDat.z = 0.0f;

   // chạy ẩu trong vòng xuyến
   if( soHoatHinh < 30 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -10.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 10.0f;
      congBezier.diemQuanTri[1].x = -11.2f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 6.8f;
      congBezier.diemQuanTri[2].x = -13.9f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 4.4f;
      congBezier.diemQuanTri[3].x = -14.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 2.0f;
      
      float thamSoBezier = (float)soHoatHinh/30.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 70 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -14.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 2.0f;
      congBezier.diemQuanTri[1].x = -14.0f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -2.4f;
      congBezier.diemQuanTri[2].x = -7.4f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -3.7f;
      congBezier.diemQuanTri[3].x = -7.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -5.6f;
      
      float thamSoBezier = ((float)soHoatHinh - 30.0f)/40.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/40.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 85 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -7.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -5.6f;
      congBezier.diemQuanTri[1].x = -6.1f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -8.1f;
      congBezier.diemQuanTri[2].x = -10.8f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -7.7f;
      congBezier.diemQuanTri[3].x = -8.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -10.9f;
      
      float thamSoBezier = ((float)soHoatHinh - 70.0f)/15.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/15.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 115 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -8.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -10.9f;
      congBezier.diemQuanTri[1].x = -5.6f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -13.7f;
      congBezier.diemQuanTri[2].x = -1.6f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -15.0f;
      congBezier.diemQuanTri[3].x = 2.1f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -14.4f;
      
      float thamSoBezier = ((float)soHoatHinh - 85.0f)/30.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 145 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 2.1f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -14.4f;
      congBezier.diemQuanTri[1].x = 6.6f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -13.8f;
      congBezier.diemQuanTri[2].x = 7.6f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -9.5f;
      congBezier.diemQuanTri[3].x = 9.3f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -7.0f;

      float thamSoBezier = ((float)soHoatHinh - 115.0f)/30.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 175 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 9.3f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -7.0f;
      congBezier.diemQuanTri[1].x = 10.9f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -4.5f;
      congBezier.diemQuanTri[2].x = 13.3f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -3.5f;
      congBezier.diemQuanTri[3].x = 13.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 0.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 145.0f)/30.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 210 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 13.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 0.0f;
      congBezier.diemQuanTri[1].x = 12.2f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 8.6f;
      congBezier.diemQuanTri[2].x = 4.4f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 11.6f;
      congBezier.diemQuanTri[3].x = 0.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 11.2f;
      
      float thamSoBezier = ((float)soHoatHinh - 175.0f)/35.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/35.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 260 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 0.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 11.2f;
      congBezier.diemQuanTri[1].x = -6.2f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 11.4f;
      congBezier.diemQuanTri[2].x = -10.9f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 4.9f;
      congBezier.diemQuanTri[3].x = -11.4f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 3.6f;
      
      float thamSoBezier = ((float)soHoatHinh - 210.0f)/50.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/50.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 270 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -11.4f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 3.6f;
      congBezier.diemQuanTri[1].x = -12.1f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 1.9f;
      congBezier.diemQuanTri[2].x = -14.0f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 0.0f;
      congBezier.diemQuanTri[3].x = -14.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 0.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 260.0f)/10.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/10.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 310 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -14.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 0.0f;
      congBezier.diemQuanTri[1].x = -14.0f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -5.0f;
      congBezier.diemQuanTri[2].x = -11.1f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -11.6f;
      congBezier.diemQuanTri[3].x = -8.2f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -8.5f;
//      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 270.0f)/40.0f );
      
      float thamSoBezier = ((float)soHoatHinh - 270.0f)/40.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/40.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 350 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -8.2f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -8.5f;
      congBezier.diemQuanTri[1].x = -5.0f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -5.3f;
      congBezier.diemQuanTri[2].x = -3.2f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -8.1f;
      congBezier.diemQuanTri[3].x = 0.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -8.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 310.0f)/40.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/40.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 390 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 0.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -8.0f;
      congBezier.diemQuanTri[1].x = 3.2f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -8.6f;
      congBezier.diemQuanTri[2].x = 12.0f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -5.8f;
      congBezier.diemQuanTri[3].x = 12.2f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -1.6f;
      
      float thamSoBezier = ((float)soHoatHinh - 350.0f)/40.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/40.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 415 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 12.2f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -1.6f;
      congBezier.diemQuanTri[1].x = 12.4f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 0.7f;
      congBezier.diemQuanTri[2].x = 10.4f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 0.2f;
      congBezier.diemQuanTri[3].x = 10.8f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 4.3f;
      
      float thamSoBezier = ((float)soHoatHinh - 390.0f)/25.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/25.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 455 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 10.8f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 4.3f;
      congBezier.diemQuanTri[1].x = 11.1f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 8.6f;
      congBezier.diemQuanTri[2].x = 5.4f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 13.1f;
      congBezier.diemQuanTri[3].x = 1.6f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 10.5f;
      
      float thamSoBezier = ((float)soHoatHinh - 415.0f)/40.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/40.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 475 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 1.6f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 10.5f;
      congBezier.diemQuanTri[1].x = 0.1f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 9.5f;
      congBezier.diemQuanTri[2].x = -0.7f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 12.4f;
      congBezier.diemQuanTri[3].x = -2.5f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 12.1f;
      
      float thamSoBezier = ((float)soHoatHinh - 455.0f)/20.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C(&congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/20.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 495 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -2.5f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 12.1f;
      congBezier.diemQuanTri[1].x = -3.4f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 11.9f;
      congBezier.diemQuanTri[2].x = -3.6;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 9.7;
      congBezier.diemQuanTri[3].x = -5.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 9.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 475.0f)/20.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/20.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 525 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -5.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 9.0f;
      congBezier.diemQuanTri[1].x = -6.6f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 8.2f;
      congBezier.diemQuanTri[2].x = -8.6f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 11.0f;
      congBezier.diemQuanTri[3].x = -10.9f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 9.1f;
      
      float thamSoBezier = ((float)soHoatHinh - 495.0f)/30.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 550 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -10.9f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 9.1f;
      congBezier.diemQuanTri[1].x = -13.6f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 6.7f;
      congBezier.diemQuanTri[2].x = -14.6f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 0.0f;
      congBezier.diemQuanTri[3].x = -14.6f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 0.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 525.0f)/25.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/25.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 585 ) {
      viTriVaiChanh.x = -14.6f;   viTriVaiChanh.y = 10.8f;   viTriVaiChanh.z = 0.0f;
      // ---- tốc độ
      tocDo = 0.0f;
   }
   else if( soHoatHinh < 610 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -14.6f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -0.0f;
      congBezier.diemQuanTri[1].x = -14.6f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -0.0f;
      congBezier.diemQuanTri[2].x = -14.5f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -3.3f;
      congBezier.diemQuanTri[3].x = -14.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -3.8f;
      
      float thamSoBezier = ((float)soHoatHinh - 585.0f)/25.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/25.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 640 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -14.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -3.8f;
      congBezier.diemQuanTri[1].x = -13.1f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -7.1f;
      congBezier.diemQuanTri[2].x = -3.9f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -3.8f;
      congBezier.diemQuanTri[3].x = 0.7f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -11.7f;
      //         printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 610.0f)/30.0f );
      float thamSoBezier = ((float)soHoatHinh - 610.0f)/30.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );

   }
   // ---- BAY LÊN TRỜI ----
   else if( soHoatHinh < 670 ) {  // ------ trái banh bay lên
      Vecto viTriBanDau;   // phải giống điểm cuối cong Bezier ở trên
      viTriBanDau.x = 0.7f;        viTriBanDau.y = 10.8f;        viTriBanDau.z = -10.7f;
      // ---- hướng bay lên kiến lồng sao gai; mục đích là (-52; 27; -52) - bính kính 4
      Vecto huongBay; // tương đối với mặt đất
      huongBay.x = -52.0f - viTriBanDau.x;
      huongBay.y = 0.0f;
      huongBay.z = -52.0f - viTriBanDau.z;
      // ---- tính điểm mục đích
      donViHoa( &huongBay );
      Vecto diemMucDich;
      diemMucDich.x = -52.0f - huongBay.x*4.0f;
      diemMucDich.y = 27.0f;
      diemMucDich.z = -52.0f - huongBay.z*4.0f;
      //      printf( "diemMucDich %5.3f %5.3f %5.3f\n", diemMucDich.x, diemMucDich.y, diemMucDich.z );
      
      Vecto buocBay;
      buocBay.x = (diemMucDich.x - viTriBanDau.x)/30.0f;
      //      buocBay.y = (diemMucDich.y - 10.8f)/30.0f;
      buocBay.z = (diemMucDich.z - viTriBanDau.z)/30.0f;
      //      printf( "buocBay %5.3f %5.3f %5.3f\n", buocBay.x, buocBay.y, buocBay.z );

      float thoiGianBay = soHoatHinh - 640.0f;
      viTriVaiChanh.x = viTriBanDau.x + buocBay.x*thoiGianBay;
      viTriVaiChanh.y = viTriBanDau.y + 1.5f*thoiGianBay + 0.5f*kHAP_DAN*thoiGianBay*thoiGianBay;
      viTriVaiChanh.z = viTriBanDau.z + buocBay.z*thoiGianBay;
      trucXoay.x = huongBay.z;
      trucXoay.y = 0.0f;
      trucXoay.z = -huongBay.x;
      tocDo = 4.0f;
   }
   else if( soHoatHinh < 691 ) {  // trái banh nhồi lại và rớt xuống
      // ---- hướng bay lên kiến lồng sao gai; mục đích là (-52; 27; -52) - bính kính 4
      Vecto huongBay;
      huongBay.x = 52.0f + 2.0f;
      huongBay.y = 0.0f;
      huongBay.z = 52.0f - 13.3f;
      // ---- tính điểm mục đích
      donViHoa( &huongBay );

      float thoiGianBay = soHoatHinh - 670.0f;
      viTriVaiChanh.x = -48.8 + 0.45f*huongBay.x*thoiGianBay;
      viTriVaiChanh.y = 27.0f + 0.45f*kHAP_DAN*thoiGianBay*thoiGianBay;
      viTriVaiChanh.z = -49.7 + 0.45f*huongBay.z*thoiGianBay;

      trucXoay.x = huongBay.z;
      trucXoay.y = 0.0f;
      trucXoay.z = -huongBay.x;
      tocDo = 4.0f;
   }
   else if( soHoatHinh < 713 ) {
      // ---- hướng bay lên kiến lồng sao gai; mục đích là (-52; 27; -52) - bính kính 4
      Vecto huongBay;
      huongBay.x = 52.0f + 2.0f;
      huongBay.y = 0.0f;
      huongBay.z = 52.0f - 13.3f;
      // ---- tính điểm mục đích
      donViHoa( &huongBay );

      float vanTocRotY = (float)(691-670)*kHAP_DAN;
      float vanTocNhoiY = -vanTocRotY*0.51f;
      float thoiGianBay = soHoatHinh - 670.0f;
      float thoiGianNhoi = soHoatHinh - 691.0f;
      viTriVaiChanh.x = -48.8 + 0.45f*huongBay.x*thoiGianBay;
      viTriVaiChanh.y = 13.9f + vanTocNhoiY*thoiGianNhoi + 0.5f*kHAP_DAN*thoiGianNhoi*thoiGianNhoi;  // mặt đất 13,2 và bán kính trái banh = 0,7
      viTriVaiChanh.z = -49.7 + 0.45f*huongBay.z*thoiGianBay;

      trucXoay.x = huongBay.z;
      trucXoay.y = 0.0f;
      trucXoay.z = -huongBay.x;
      donViHoa( &trucXoay );
      tocDo = 3.0f;
   }
   // ---- CHỐNG MẶT đi lang thang
   else if( soHoatHinh < 720 ) {  // trái banh rớt xuống và đi va chạm
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -33.4f;   congBezier.diemQuanTri[0].y = 13.3f;   congBezier.diemQuanTri[0].z = -38.4f;
      congBezier.diemQuanTri[1].x = -32.3f;   congBezier.diemQuanTri[1].y = 13.3f;   congBezier.diemQuanTri[1].z = -38.0f;
      congBezier.diemQuanTri[2].x = -31.7f;   congBezier.diemQuanTri[2].y = 13.3f;   congBezier.diemQuanTri[2].z = -37.7f;
      congBezier.diemQuanTri[3].x = -31.0f;   congBezier.diemQuanTri[3].y = 13.3f;   congBezier.diemQuanTri[3].z = -36.7f;
      //      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 270.0f)/40.0f );
      
      float thamSoBezier = ((float)soHoatHinh - 713.0f)/6.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/6.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 740 ) {  // nhồi lại từ va chạm
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -31.0f;   congBezier.diemQuanTri[0].y = 13.3f;   congBezier.diemQuanTri[0].z = -36.7f;
      congBezier.diemQuanTri[1].x = -32.5f;   congBezier.diemQuanTri[1].y = 13.3f;   congBezier.diemQuanTri[1].z = -37.7f;
      congBezier.diemQuanTri[2].x = -35.7f;   congBezier.diemQuanTri[2].y = 13.3f;   congBezier.diemQuanTri[2].z = -38.4f;
      congBezier.diemQuanTri[3].x = -37.3f;   congBezier.diemQuanTri[3].y = 13.3f;   congBezier.diemQuanTri[3].z = -37.1f;
      //      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 270.0f)/40.0f );
      
      float thamSoBezier = ((float)soHoatHinh - 720.0f)/20.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/20.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 780 ) {  // trái banh nhồi lại và rớt xuống
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -37.3f;   congBezier.diemQuanTri[0].y = 13.3f;   congBezier.diemQuanTri[0].z = -37.1f;
      congBezier.diemQuanTri[1].x = -40.1f;   congBezier.diemQuanTri[1].y = 13.3f;   congBezier.diemQuanTri[1].z = -34.8f;
      congBezier.diemQuanTri[2].x = -33.4f;   congBezier.diemQuanTri[2].y = 13.3f;   congBezier.diemQuanTri[2].z = -35.3f;
      congBezier.diemQuanTri[3].x = -31.8f;   congBezier.diemQuanTri[3].y = 13.3f;   congBezier.diemQuanTri[3].z = -38.9f;
      //      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 740.0f)/40.0f );
      
      float thamSoBezier = ((float)soHoatHinh - 740.0f)/40.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/40.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 860 ) {  // trái banh nhồi lại và rớt xuống
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -31.8f;   congBezier.diemQuanTri[0].y = 13.3f;   congBezier.diemQuanTri[0].z = -38.9f;
      congBezier.diemQuanTri[1].x = -30.2f;   congBezier.diemQuanTri[1].y = 13.3f;   congBezier.diemQuanTri[1].z = -42.7f;
      congBezier.diemQuanTri[2].x = -31.8f;   congBezier.diemQuanTri[2].y = 13.3f;   congBezier.diemQuanTri[2].z = -45.4f;
      congBezier.diemQuanTri[3].x = -30.8f;   congBezier.diemQuanTri[3].y = 13.3f;   congBezier.diemQuanTri[3].z = -51.5f;
      //      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 780.0f)/80.0f );

      float thamSoBezier = ((float)soHoatHinh - 780.0f)/80.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/80.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      
      vaiChanh->thaiTrang = kVAI_CHANH__THAI_TRANG_CHAY_TANG_1;
   }
   
   // ---- CHẠY CHỐNG
   else if( soHoatHinh < 920 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -30.8f;   congBezier.diemQuanTri[0].y = 13.3f;   congBezier.diemQuanTri[0].z = -51.5f;
      congBezier.diemQuanTri[1].x = -29.6f;   congBezier.diemQuanTri[1].y = 13.3f;   congBezier.diemQuanTri[1].z = -52.7f;
      congBezier.diemQuanTri[2].x = -7.1f;   congBezier.diemQuanTri[2].y = 13.3f;   congBezier.diemQuanTri[2].z = -52.0f;
      congBezier.diemQuanTri[3].x = -7.1f;   congBezier.diemQuanTri[3].y = 13.3f;   congBezier.diemQuanTri[3].z = -52.0f;
      //      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 860.0f)/40.0f );
      
      float thamSoBezier = ((float)soHoatHinh - 860.0f)/60.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/60.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      
      // ---- xuống tầng 0
      Vecto cachTamLong;  // cách tâm lồng
      cachTamLong.x = viTriVaiChanh.x + 52.0f - 0.7f*0.7f;
      cachTamLong.z = viTriVaiChanh.z + 52.0f - 0.7f*0.7f;  // 0,7 bán kính vai chánh * √2
      float quangBinh = cachTamLong.x*cachTamLong.x + cachTamLong.z*cachTamLong.z; // quãng bình
      if( vaiChanh->thaiTrang == kVAI_CHANH__THAI_TRANG_CHAY_TANG_1 ) {
         if( quangBinh > 676.0f ) {
            vaiChanh->thaiTrang = kVAI_CHANH__THAI_TRANG_NHAY_XUONG_1_0;
            vaiChanh->vanToc.y = 0.0f;
         }
      }
      else if( vaiChanh->thaiTrang == kVAI_CHANH__THAI_TRANG_NHAY_XUONG_1_0 ) {
         vaiChanh->vanToc.y += kHAP_DAN;
         viTriVaiChanh.y = vaiChanh->dich[13] + vaiChanh->vanToc.y;
         if( viTriVaiChanh.y < 13.0f ) {
            viTriVaiChanh.y = 13.0f;
            vaiChanh->thaiTrang = kVAI_CHANH__THAI_TRANG_CHAY_TANG_0;
         }
      }
      else if( vaiChanh->thaiTrang == kVAI_CHANH__THAI_TRANG_CHAY_TANG_0 ) {
         viTriVaiChanh.y = 13.0f;
         if( quangBinh > 1024.0f ) {
            vaiChanh->thaiTrang = kVAI_CHANH__THAI_TRANG_NHAY_XUONG_0_SAN;
 
            vaiChanh->vanToc.y = 0.0f;
         }
      }
      else if( vaiChanh->thaiTrang == kVAI_CHANH__THAI_TRANG_NHAY_XUONG_0_SAN ) {
         vaiChanh->vanToc.y += kHAP_DAN;
         viTriVaiChanh.y = vaiChanh->dich[13] + vaiChanh->vanToc.y;
         if( viTriVaiChanh.y < 12.7f ) {
            viTriVaiChanh.y = 12.7f;
            vaiChanh->thaiTrang = kVAI_CHANH__THAI_TRANG_CHAY_SAN;
         }
      }
      else if( vaiChanh->thaiTrang == kVAI_CHANH__THAI_TRANG_CHAY_SAN ) {
         viTriVaiChanh.y = 12.7f;
         if( viTriVaiChanh.x > 45.0f - 52.0f + 0.7f*0.7f ) {
            vaiChanh->thaiTrang = kVAI_CHANH__THAI_TRANG_NHAY_XUONG_SAN_DAT;
            vaiChanh->vanToc.y = 0.0f;
         }
      }

   }
   else if( soHoatHinh < 940 ) {  // đứng trước nhảy xuống
      viTriVaiChanh.x = -7.1f;
      viTriVaiChanh.y = 12.7f;
      viTriVaiChanh.z = -52.0f;
      tocDo = 0.0f;
   }
   else if( soHoatHinh < 952 ) { // nhảy xuống
      viTriVaiChanh.x = -7.1f + 0.4f*(soHoatHinh - 940);
      viTriVaiChanh.y = 12.7f;
      viTriVaiChanh.z = -52.0f;

      trucXoay.x = 0.0f;   trucXoay.y = 0.0f;   trucXoay.z = -1.0f;
      tocDo = 0.4f;

      if( vaiChanh->thaiTrang == kVAI_CHANH__THAI_TRANG_CHAY_SAN ) {
         if( viTriVaiChanh.x > 45.0f - 52.0f + 0.7f*0.7f ) {
            vaiChanh->thaiTrang = kVAI_CHANH__THAI_TRANG_NHAY_XUONG_SAN_DAT;
            vaiChanh->vanToc.y = 0.0f;
         }
      }
      else if( vaiChanh->thaiTrang == kVAI_CHANH__THAI_TRANG_NHAY_XUONG_SAN_DAT ) {
         vaiChanh->vanToc.y += kHAP_DAN;
         viTriVaiChanh.y = vaiChanh->dich[13] + vaiChanh->vanToc.y;
         if( viTriVaiChanh.y < 10.8f ) {
            viTriVaiChanh.y = 10.8f;
            vaiChanh->thaiTrang = kVAI_CHANH__THAI_TRANG_CHAY_DAT;
         }
      }
      else if( vaiChanh->thaiTrang == kVAI_CHANH__THAI_TRANG_CHAY_DAT ) {
         viTriVaiChanh.y = 10.8f;
      }
   }
   else if( soHoatHinh < 990 ) {  // CHẠY TRÊN ĐƯỜNG
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -2.3f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -52.0f;
      congBezier.diemQuanTri[1].x = -1.3f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -50.5f;
      congBezier.diemQuanTri[2].x = -2.1f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -40.6f;
      congBezier.diemQuanTri[3].x = -2.2f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -32.0f;
      //      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 740.0f)/40.0f );
      
      float thamSoBezier = ((float)soHoatHinh - 952.0f)/48.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/48.0f;
//      printf( "%d vaiChanh tocDo %5.3f\n", soHoatHinh, tocDo );
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 1030 ) {  // CHẠY TRÊN ĐƯỜNG
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -2.2f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -32.0f;
      congBezier.diemQuanTri[1].x = -2.3f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -20.4f;
      congBezier.diemQuanTri[2].x = -1.0f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -12.0f;
      congBezier.diemQuanTri[3].x = -5.7f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -9.6f;
      
      float thamSoBezier = ((float)soHoatHinh - 990.0f)/40.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/40.0f;
//      printf( "%d vaiChanh tocDo %5.3f\n", soHoatHinh, tocDo );
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 1090 ) {  // CHẠY TRÊN ĐƯỜNG
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = -5.7f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -9.6f;
      congBezier.diemQuanTri[1].x = -16.8f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -3.8f;
      congBezier.diemQuanTri[2].x = -11.6f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 11.1f;
      congBezier.diemQuanTri[3].x = 0.0f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = 11.1f;

      float thamSoBezier = ((float)soHoatHinh - 1030.0f)/60.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/60.0f;
//      printf( "%d vaiChanh tocDo %5.3f\n", soHoatHinh, tocDo );
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 1135 ) {  // CHẠY TRÊN ĐƯỜNG
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 0.0f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = 11.1f;
      congBezier.diemQuanTri[1].x = 6.8f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = 11.1f;
      congBezier.diemQuanTri[2].x = 13.7f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = 0.5f;
      congBezier.diemQuanTri[3].x = 6.1f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -6.5f;
       
      float thamSoBezier = ((float)soHoatHinh - 1090.0f)/45.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
//      printf( "%d vaiChanh tocDo %5.3f\n", soHoatHinh, tocDo );
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 1145 ) {  // CHẠY TRÊN ĐƯỜNG
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 6.1f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -6.5f;
      congBezier.diemQuanTri[1].x = 4.9f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -7.7f;
      congBezier.diemQuanTri[2].x = 3.2f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -8.3f;
      congBezier.diemQuanTri[3].x = 2.8f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -9.0f;
      //      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 740.0f)/40.0f );
      
      float thamSoBezier = ((float)soHoatHinh - 1135.0f)/15.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/15.0f;
//      printf( "%d vaiChanh tocDo %5.3f\n", soHoatHinh, tocDo );
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 1170 ) {  // CHẠY TRÊN ĐƯỜNG
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 2.8f;   congBezier.diemQuanTri[0].y = 10.8f;   congBezier.diemQuanTri[0].z = -9.0f;
      congBezier.diemQuanTri[1].x = 2.2f;   congBezier.diemQuanTri[1].y = 10.8f;   congBezier.diemQuanTri[1].z = -12.5f;
      congBezier.diemQuanTri[2].x = 1.7f;   congBezier.diemQuanTri[2].y = 10.8f;   congBezier.diemQuanTri[2].z = -18.0f;
      congBezier.diemQuanTri[3].x = 1.7f;   congBezier.diemQuanTri[3].y = 10.8f;   congBezier.diemQuanTri[3].z = -28.0f;
//      printf("soHoatHinh %d  %5.3f\n", soHoatHinh, ((float)soHoatHinh - 740.0f)/40.0f );
      
      float thamSoBezier = ((float)soHoatHinh - 1145.0f)/25.0f;
      viTriVaiChanh = tinhViTriBezier3C(&congBezier, thamSoBezier );
      // ---- vận tốc
      vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/25.0f;
//      printf( "%d vaiChanh tocDo %5.3f\n", soHoatHinh, tocDo );
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
   }
   else if( soHoatHinh < 1600 ) {  // CHẠY TRÊN ĐƯỜNG Z

      viTriVaiChanh.x = 1.7f;
      viTriVaiChanh.y = 10.8f;
      viTriVaiChanh.z = -28.0f + 1.3f*(1170 - soHoatHinh);

      trucXoay.x = -1.0f;
      trucXoay.y = 0.0f;
      trucXoay.z = 0.0f;
      tocDo = 0.6f;
   }
   
   
//   printf( "---> %d viTriVaiChanh %d  %5.3f %5.3f %5.3f <---\n", soHoatHinh, vaiChanh->thaiTrang, viTriVaiChanh.x, viTriVaiChanh.y, viTriVaiChanh.z );
   xoay = tinhXoayChoVatThe( vaiChanh, trucXoay, tocDo, 0.7f );
   // ---- đặt biến hóa cho vai chánh
   datBienHoaChoVat( vaiChanh, &phongTo, &xoay, &viTriVaiChanh );
}

#define kSAO_GAI__BUOC_RA_LONG 0.14f
#define kSAO_GAI__BAN_KINH     1.5f
#define kSAO_GAI__BUOC_RUOT 0.4f

#define kSAO_GAI__THAI_TRANG_DANG_DOI   0
#define kSAO_GAI__THAI_TRANG_THA_XUONG  1
#define kSAO_GAI__THAI_TRANG_RA_LONG    2
#define kSAO_GAI__THAI_TRANG_ROT_XUONG_LONG 3
#define kSAO_GAI__THAI_TRANG_NGO            4
#define kSAO_GAI__THAI_TRANG_CHAY_TANG_4    5
#define kSAO_GAI__THAI_TRANG_NHAY_XUONG_4_3 6
#define kSAO_GAI__THAI_TRANG_CHAY_TANG_3    7
#define kSAO_GAI__THAI_TRANG_NHAY_XUONG_3_2 8
#define kSAO_GAI__THAI_TRANG_CHAY_TANG_2    9
#define kSAO_GAI__THAI_TRANG_NHAY_XUONG_2_1 10
#define kSAO_GAI__THAI_TRANG_CHAY_TANG_1    11
#define kSAO_GAI__THAI_TRANG_NHAY_XUONG_1_0 12
#define kSAO_GAI__THAI_TRANG_CHAY_TANG_0    13
#define kSAO_GAI__THAI_TRANG_NHAY_XUONG_0_SAN 14
#define kSAO_GAI__THAI_TRANG_CHAY_SAN       15
#define kSAO_GAI__THAI_TRANG_NHAY_XUONG_SAN_DAT    16

#define kSAO_GAI__THAI_TRANG_DUONG_VAO_VONG 17
#define kSAO_GAI__THAI_TRANG_DUONG_RA_VONG  18

void nangCapSaoGai( VatThe *saoGai, unsigned short soHoatHinh ) {
   
   Vecto phongTo;
   phongTo.x = saoGai->phongTo[0];   phongTo.y = saoGai->phongTo[5];   phongTo.z = saoGai->phongTo[10];
   
   Quaternion xoay;
   xoay.w = 1.0f;   xoay.x = 0.0f;   xoay.y = 0.0f;   xoay.z = 0.0f;

   Vecto viTriSaoGai;
   viTriSaoGai.x = -52.0f;   viTriSaoGai.y = 27.7f;    viTriSaoGai.z = -52.0f;  // cần có gia trị sẵn cho không có giá trị bậy
   
   float banKinh = kSAO_GAI__BAN_KINH*phongTo.x;
   Vecto trucXoay;
   float tocDo = 0.0f;
   
   unsigned char thaiTrangSaoGai = saoGai->thaiTrang;
   // ----- sao gai thoát lồng
   if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_DANG_DOI ) {
      if( soHoatHinh == 700 )
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_THA_XUONG;
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_THA_XUONG ) {
      float buocXuong = 0.01f;

      viTriSaoGai.x = -52.0f;
      viTriSaoGai.y = saoGai->dich[13] - buocXuong;
      viTriSaoGai.z = -52.0f;
      
      datBienHoaChoVat( saoGai, &phongTo, &xoay, &viTriSaoGai );
      if( saoGai->dich[13] < 27.2f ) {
         viTriSaoGai.y = 27.2f;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_RA_LONG;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_RA_LONG ) {

      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RA_LONG;
      viTriSaoGai.y = 27.2f;
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RA_LONG;  // 1.8f bán kính sao gai

      if( saoGai->dich[12] > -52.0f + 4.5f ) {
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_ROT_XUONG_LONG;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_ROT_XUONG_LONG ) {
      saoGai->vanToc.y += kHAP_DAN;

      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RA_LONG;
      viTriSaoGai.y = saoGai->dich[13] + saoGai->vanToc.y;
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RA_LONG;  // 1.8f bán kính sao gai

      if( viTriSaoGai.y < 13.5f + banKinh ) {
         viTriSaoGai.y = 13.5f + banKinh;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_NGO;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_NGO ) {  // ngó
      
      viTriSaoGai.x = saoGai->dich[12] + 0.05f;
      viTriSaoGai.y = 13.5f + banKinh;  // 13,5 là độ cao mặt tầng 4
      viTriSaoGai.z = -52.0f;

      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RA_LONG;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[12] > -52.0f + 10.5f ) {
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_CHAY_TANG_4;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_CHAY_TANG_4 ) {

      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = 13.5f + banKinh;  // 13,5 là độ cao mặt tầng 4
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai

      if( saoGai->dich[12] > -52.0f + 11.0f + 0.5f*banKinh ) {
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_NHAY_XUONG_4_3;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_NHAY_XUONG_4_3 ) {
      saoGai->vanToc.y += kHAP_DAN;
   
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = saoGai->dich[13] + saoGai->vanToc.y;
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[13] < 13.2f + banKinh ) {
         viTriSaoGai.y = 13.2f + banKinh;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_CHAY_TANG_3;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_CHAY_TANG_3 ) {
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = 13.2f + banKinh;  // 13,5 là độ cao mặt tầng 4
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[12] > -52.0f + 15.0f + 0.5f*banKinh ) {
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_NHAY_XUONG_3_2;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_NHAY_XUONG_3_2 ) {
      saoGai->vanToc.y += kHAP_DAN;
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = saoGai->dich[13] + saoGai->vanToc.y;
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[13] < 12.9f + banKinh ) {
         viTriSaoGai.y = 12.9f + banKinh;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_CHAY_TANG_2;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_CHAY_TANG_2 ) {
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = 12.9 + banKinh;  // 12,9 là độ cao mặt tầng 4
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[12] > -52.0f + 20.0f + 0.5f*banKinh ) {
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_NHAY_XUONG_2_1;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_NHAY_XUONG_2_1 ) {
      saoGai->vanToc.y += kHAP_DAN;
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = saoGai->dich[13] + saoGai->vanToc.y;
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[13] < 12.6f + banKinh ) {
         viTriSaoGai.y = 12.6f + banKinh;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_CHAY_TANG_1;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_CHAY_TANG_1 ) {
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = 12.6f + banKinh;  // 13,5 là độ cao mặt tầng 4
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[12] > -52.0f + 26.0f + 0.5f*banKinh ) {
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_NHAY_XUONG_1_0;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_NHAY_XUONG_1_0 ) {
      saoGai->vanToc.y += kHAP_DAN;
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = saoGai->dich[13] + saoGai->vanToc.y;
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[13] < 12.3f + banKinh ) {
         viTriSaoGai.y = 12.3f + banKinh;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_CHAY_TANG_0;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_CHAY_TANG_0 ) {
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = 12.3f + banKinh;  // 13,5 là độ cao mặt tầng 4
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[12] > -52.0f + 32.0f + 0.5f*banKinh ) {
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_NHAY_XUONG_0_SAN;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_NHAY_XUONG_0_SAN ) {
      saoGai->vanToc.y += kHAP_DAN;
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = saoGai->dich[13] + saoGai->vanToc.y;
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[13] < 12.0f + banKinh ) {
         viTriSaoGai.y = 12.0f + banKinh;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_CHAY_SAN;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_CHAY_SAN ) {
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = 12.0f + banKinh;  // 13,5 là độ cao mặt tầng 4
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( saoGai->dich[12] > -52.0f + 45.0f + 0.5f*kSAO_GAI__BAN_KINH ) {
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_NHAY_XUONG_SAN_DAT;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( thaiTrangSaoGai == kSAO_GAI__THAI_TRANG_NHAY_XUONG_SAN_DAT ) {
      saoGai->vanToc.y += kHAP_DAN;
      
      viTriSaoGai.x = saoGai->dich[12] + kSAO_GAI__BUOC_RUOT;
      viTriSaoGai.y = saoGai->dich[13] + saoGai->vanToc.y;
      viTriSaoGai.z = -52.0f;
      
      trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = -1.0f;
      tocDo = kSAO_GAI__BUOC_RUOT;  // 1.8f bán kính sao gai
      
      if( viTriSaoGai.y < 10.1f + banKinh ) {
         viTriSaoGai.y = 10.1f + banKinh;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_DUONG_VAO_VONG;
         saoGai->vanToc.y = 0.0f;
      }
   }
   else if( (soHoatHinh > 961) && (soHoatHinh < 982) ) {
      viTriSaoGai.x = -2.64f;
      viTriSaoGai.y = 10.1f + banKinh;
      viTriSaoGai.z = -52.0f;
   }
   else if( soHoatHinh > 981 ) { // xuống đường lộ
      float toaDoY = 10.1 + banKinh;  // cho chạy trên mặt đường lộ

      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f;    phapThuyenMatDat.y = 1.0f;    phapThuyenMatDat.z = 0.0f;

      if( soHoatHinh < 1010 ) {

         if( soHoatHinh > 990 ) {  // khi ăn trái banh xui
            float tiSoPhongTo = 1.0f + 0.02f*(soHoatHinh - 990);
            phongTo.x = tiSoPhongTo;
            phongTo.y = tiSoPhongTo;
            phongTo.z = tiSoPhongTo;
         }
         viTriSaoGai.x = -2.64f;
         viTriSaoGai.y = 10.1f + banKinh;
         viTriSaoGai.z = -52.0f;
      }
      else if( soHoatHinh < 1050 ) {
         
         Bezier congBezier;  // cong Bezier cho đoạn thời gian này
         congBezier.diemQuanTri[0].x = -2.64f;   congBezier.diemQuanTri[0].y = toaDoY;   congBezier.diemQuanTri[0].z = -52.0f;
         congBezier.diemQuanTri[1].x = -2.64f;   congBezier.diemQuanTri[1].y = toaDoY;   congBezier.diemQuanTri[1].z = -52.0f;
         congBezier.diemQuanTri[2].x = -2.8f;   congBezier.diemQuanTri[2].y = toaDoY;   congBezier.diemQuanTri[2].z = -30.0f;
         congBezier.diemQuanTri[3].x = -3.2f;   congBezier.diemQuanTri[3].y = toaDoY;   congBezier.diemQuanTri[3].z = -16.9f;
         
         float thamSoBezier = ((float)soHoatHinh - 1010)/40.0f;
         viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );

         // ---- vận tốc
         Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
         tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/89.0f;
//         printf( "%d  saoGai tocDo %5.3f  thamSoBezier %5.3f\n", soHoatHinh, tocDo, thamSoBezier );
         trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      }
      else if( soHoatHinh < 1085 ) {
         Bezier congBezier;  // cong Bezier cho đoạn thời gian này
         congBezier.diemQuanTri[0].x = -3.2f;   congBezier.diemQuanTri[0].y = toaDoY;   congBezier.diemQuanTri[0].z = -16.9f;
         congBezier.diemQuanTri[1].x = -3.5f;   congBezier.diemQuanTri[1].y = toaDoY;   congBezier.diemQuanTri[1].z = -8.1f;
         congBezier.diemQuanTri[2].x = -9.4f;   congBezier.diemQuanTri[2].y = toaDoY;   congBezier.diemQuanTri[2].z = -6.7f;
         congBezier.diemQuanTri[3].x = -9.4f;   congBezier.diemQuanTri[3].y = toaDoY;   congBezier.diemQuanTri[3].z = 0.0f;
         
         float thamSoBezier = ((float)soHoatHinh - 1050)/35.0f;
         viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
         // ---- vận tốc
         Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
         tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/35.0f;
//         printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
         trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      }
      else if( soHoatHinh < 1115 ) {
         Bezier congBezier;  // cong Bezier cho đoạn thời gian này
         congBezier.diemQuanTri[0].x = -9.4f;   congBezier.diemQuanTri[0].y = toaDoY;   congBezier.diemQuanTri[0].z = 0.0f;
         congBezier.diemQuanTri[1].x = -9.4f;   congBezier.diemQuanTri[1].y = toaDoY;   congBezier.diemQuanTri[1].z = 5.9f;
         congBezier.diemQuanTri[2].x = -4.8f;   congBezier.diemQuanTri[2].y = toaDoY;   congBezier.diemQuanTri[2].z = 9.8f;
         congBezier.diemQuanTri[3].x = 0.0f;   congBezier.diemQuanTri[3].y = toaDoY;   congBezier.diemQuanTri[3].z = 9.8f;
         
         float thamSoBezier = ((float)soHoatHinh - 1085)/30.0f;
         viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
         // ---- vận tốc
         Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
         tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
//         printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
         trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      }
      else if( soHoatHinh < 1145 ) {
         Bezier congBezier;  // cong Bezier cho đoạn thời gian này
         congBezier.diemQuanTri[0].x = 0.0f;   congBezier.diemQuanTri[0].y = toaDoY;   congBezier.diemQuanTri[0].z = 9.8f;
         congBezier.diemQuanTri[1].x = 4.8f;   congBezier.diemQuanTri[1].y = toaDoY;   congBezier.diemQuanTri[1].z = 9.8f;
         congBezier.diemQuanTri[2].x = 9.9f;   congBezier.diemQuanTri[2].y = toaDoY;   congBezier.diemQuanTri[2].z = 5.3f;
         congBezier.diemQuanTri[3].x = 9.9f;   congBezier.diemQuanTri[3].y = toaDoY;   congBezier.diemQuanTri[3].z = 0.0f;
         
         float thamSoBezier = ((float)soHoatHinh - 1115)/30.0f;
         viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
         // ---- vận tốc
         Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
         tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
//         printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
         trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      }
      else if( soHoatHinh < 1175 ) {
         Bezier congBezier;  // cong Bezier cho đoạn thời gian này
         congBezier.diemQuanTri[0].x = 9.9f;   congBezier.diemQuanTri[0].y = toaDoY;   congBezier.diemQuanTri[0].z = 0.0f;
         congBezier.diemQuanTri[1].x = 9.9f;   congBezier.diemQuanTri[1].y = toaDoY;   congBezier.diemQuanTri[1].z = -5.3f;
         congBezier.diemQuanTri[2].x = 3.0f;   congBezier.diemQuanTri[2].y = toaDoY;   congBezier.diemQuanTri[2].z = -12.7f;
         congBezier.diemQuanTri[3].x = 3.0f;   congBezier.diemQuanTri[3].y = toaDoY;   congBezier.diemQuanTri[3].z = -26.0f;
         
         float thamSoBezier = ((float)soHoatHinh - 1145)/30.0f;
         viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
         // ---- vận tốc
         Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
         tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/30.0f;
//         printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
         trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      }
      else if( soHoatHinh < 1600 ) {
         viTriSaoGai.x = 3.0f;
         viTriSaoGai.y = toaDoY;
         viTriSaoGai.z = -26.0 - 1.3f*(soHoatHinh - 1175);
         trucXoay.x = -1.0;
         trucXoay.y = 0.0;
         trucXoay.z = 0.0;
         donViHoa( &trucXoay );
         tocDo = 0.6f;
         saoGai->thaiTrang = kSAO_GAI__THAI_TRANG_DUONG_RA_VONG;
      }
   }

   xoay = tinhXoayChoVatThe( saoGai, trucXoay, tocDo, banKinh );
   datBienHoaChoVat( saoGai, &phongTo, &xoay, &viTriSaoGai );
//   printf( "-----> %d vi tri saoGai  %5.3f %5.3f %5.3f  phongTo %5.3f\n", soHoatHinh, viTriSaoGai.x, viTriSaoGai.y, viTriSaoGai.z, phongTo.x );
}


void nangCapTraiBanhBiGiet( VatThe *traiBanh, unsigned short soHoatHinh ) {
   
   // ---- đi thẳng lên đường z đến nơi gặp sai gai và chết
   Vecto phongTo;
   phongTo.x = 1.0f;   phongTo.y = 1.0f;   phongTo.z = 1.0f;

   Vecto trucXoay;
   trucXoay.x = 1.0f;   trucXoay.y = 0.0f;   trucXoay.z = 0.0f;

   Quaternion xoay;

   Vecto viTriTraiBanh;
   viTriTraiBanh.x = -3.0f;
   viTriTraiBanh.y = 10.1f + traiBanh->hinhDang.hinhCau.banKinh;
   viTriTraiBanh.z = -535.0f;
   float tocDo = 0.0f;
   
   float banKinhTraiBanh = traiBanh->hinhDang.hinhCau.banKinh;
   
   if( soHoatHinh < 965 ) {  // chạy trên lộ bình thường
      viTriTraiBanh.z = -535.0f + kTOC_DO_TRAI_BANH_PHIM_TRUONG0*soHoatHinh;
      tocDo = kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
   }
   else if( soHoatHinh < 970 ) {  // chặm lại khi gặp trái sao gai
      float goc = 6.2831852f*(soHoatHinh - 965)/20.0f;
      viTriTraiBanh.z = -535.0 + kTOC_DO_TRAI_BANH_PHIM_TRUONG0*(965 + 5.0f*sinf( goc )/6.2831852f);
      tocDo = cosf( goc );
   }
   else if( soHoatHinh < 990 ) {  // rung
      float goc = 6.2831852f*(970 - 965)/20.0f;
      viTriTraiBanh.z = -535.0 + kTOC_DO_TRAI_BANH_PHIM_TRUONG0*(965 + 5.0f*sinf( goc )/6.2831852f);
      tocDo = cosf( goc );
      // ---- đổi bán kính
      if( soHoatHinh & 0x02 )
         traiBanh->hinhDang.hinhCau.banKinh += 0.1f;
      else
         traiBanh->hinhDang.hinhCau.banKinh -= 0.1f;
   }
   else if( soHoatHinh < 1010 ) {
      float goc = 6.2831852f*(970 - 965)/20.0f;
      viTriTraiBanh.z = -535.0 + kTOC_DO_TRAI_BANH_PHIM_TRUONG0*(965 + 5.0f*sinf( goc )/6.2831852f);
      tocDo = 0.0f;
      // ---- đừng cho bạn kính = 0
      if( banKinhTraiBanh > 0.1f )
         traiBanh->hinhDang.hinhCau.banKinh -= 0.1f;
   }

   xoay = tinhXoayChoVatThe( traiBanh, trucXoay, tocDo, banKinhTraiBanh );
   datBienHoaChoVat( traiBanh, &phongTo, &xoay, &viTriTraiBanh );
//   printf( "---> %d vi tri traiBanhBiGiet  %5.3f %5.3f %5.3f\n", soHoatHinh, viTriTraiBanh.x, viTriTraiBanh.y, viTriTraiBanh.z );

}

void nangCapKienLongSaoGai( VatThe *kienLongSaoGai, unsigned short soHoatHinh ) {
   
   Vecto phongTo;
   phongTo.x = 1.0f;   phongTo.y = 1.0f;   phongTo.z = 1.0f;
   
   Vecto trucXoay;
   trucXoay.x = 0.0f;   trucXoay.y = 1.0f;   trucXoay.z = 0.0f;
   
   Quaternion xoay = datQuaternionTuVectoVaGocQuay( &trucXoay, 0.31415926f*0.1f*soHoatHinh );
   
   Vecto viTriKienLongSaoGai;
   viTriKienLongSaoGai.x = -52.0f;   viTriKienLongSaoGai.y = 27.7f;   viTriKienLongSaoGai.z = -52.0f;
   
   // ---- kiến lồng sao gai
   if( soHoatHinh < 675 ) {
      ; // ---- không làm gì đặt biệt
   }
   else if( soHoatHinh < 750 ) { // mở kiến nhốp sao gai
      
      float buocKeoLen = 3.2f/(750.0f - 675.0f);
      unsigned short soHoatHinhTuongDoi = soHoatHinh - 675.0f;
      // ---- nâng cấp bề cao kiến
      unsigned short chiSo = 0;
      unsigned short soLuongVatThe = kienLongSaoGai->soLuongVatThe;
      while( chiSo < soLuongVatThe ) {
         float beCao = 3.5f - buocKeoLen*soHoatHinhTuongDoi;
         kienLongSaoGai->danhSachVatThe[chiSo].hinhDang.hop = datHop( 1.0f, beCao, 0.2f, &(kienLongSaoGai->danhSachVatThe[chiSo].baoBiVT) );
         chiSo++;
      }
      
      // ---- đừng quên tính bao bì vật thể cho vật thể ghép
      tinhBaoBiVTChoVatTheGhep( kienLongSaoGai );
      
      // ---- nâng cấp vị trí
      viTriKienLongSaoGai.x = -52.0f;
      viTriKienLongSaoGai.y = 27.7f + 0.5f*buocKeoLen*soHoatHinhTuongDoi;
      viTriKienLongSaoGai.z = -52.0f;
//      printf( "beCao %5.3f   viTri %5.3f %5.3f %5.3f  buocKeoLen %5.3f\n", 3.5f - buocKeoLen*(soHoatHinh - 675.0f), viTriKienLongSaoGai.x, viTriKienLongSaoGai.y, viTriKienLongSaoGai.z, buocKeoLen );
   }
   else {  // phải làm này vì không biết cần bắt đầu từ bức ảnh nào
      
      unsigned short chiSo = 0;
      unsigned short soLuongVatThe = kienLongSaoGai->soLuongVatThe;
      while( chiSo < soLuongVatThe ) {
         kienLongSaoGai->danhSachVatThe[chiSo].hinhDang.hop = datHop( 1.0f, 0.3f, 0.2f, &(kienLongSaoGai->danhSachVatThe[chiSo].baoBiVT) );
         chiSo++;
      }
      
      // ---- đừng quên tính bao bì vật thể cho vật thể ghép
      tinhBaoBiVTChoVatTheGhep( kienLongSaoGai );
      
      viTriKienLongSaoGai.x = -52.0f;
      viTriKienLongSaoGai.y = 27.7f + 1.6f;
      viTriKienLongSaoGai.z = -52.0f;
   }
   
   datBienHoaChoVat( kienLongSaoGai, &phongTo, &xoay, &viTriKienLongSaoGai );
}


void nangCapHatBayQuanhVongXuyen( VatThe *hatBayQuanh, unsigned short soHoatHinh ) {

   Vecto phongTo;
   phongTo.x = 1.0f;   phongTo.y = 1.0f;   phongTo.z = 1.0f;
   
   Vecto trucXoay;
   trucXoay.x = 0.0f;   trucXoay.y = 1.0f;   trucXoay.z = 0.0f;
   
   Quaternion xoay = datQuaternionTuVectoVaGocQuay( &trucXoay, 0.31415926f*0.4f*soHoatHinh );

   datXoayChoVat( hatBayQuanh, &xoay );
   tinhBaoBiVTChoVatTheGhep( hatBayQuanh );
   gomBienHoaChoVat( hatBayQuanh );
}

void nangCapBongBongBay( VatThe *danhSachBongBong, unsigned short soLuongBongBong, unsigned short soHoatHinh ) {

   Vecto phongTo;
   phongTo.x = 1.0f;   phongTo.y = 1.0f;   phongTo.z = 1.0f;
   
   Vecto trucXoay;
   trucXoay.x = 0.0f;   trucXoay.y = 1.0f;   trucXoay.z = 0.0f;
   
   Quaternion xoay;
   xoay.w = 1.0f;    xoay.x = 0.0f;     xoay.y = 0.0f;     xoay.z = 0.0f;
   
   Vecto viTriBongBong;

   unsigned short chiSo = 0;
   while( chiSo < soLuongBongBong ) {
      // ---- xài vị trí đầu cho bong bóng di chuyển hơi khác nhau
      Vecto viTriDau = danhSachBongBong[chiSo].viTriDau;
      viTriBongBong.x = danhSachBongBong[chiSo].viTriDau.x + 2.0f*sinf( soHoatHinh*0.1f + viTriDau.x ) + 0.6f*soHoatHinh;
      viTriBongBong.y = danhSachBongBong[chiSo].viTriDau.y + 2.0f*cosf( soHoatHinh*0.1f + viTriDau.y );
      viTriBongBong.z = danhSachBongBong[chiSo].viTriDau.z + sinf( soHoatHinh*0.1f + viTriDau.z ) - soHoatHinh;

      datBienHoaChoVat( &(danhSachBongBong[chiSo]), &phongTo, &xoay, &viTriBongBong );

      chiSo++;
   }
}

void nangCapCacBatDienXoay( VatThe *danhSachBatDien, unsigned short soLuongBatDien, unsigned short soHoatHinh ) {
   
   Vecto phongTo;
   phongTo.x = 1.0f;   phongTo.y = 1.0f;   phongTo.z = 1.0f;
   
   Vecto trucXoay;
   trucXoay.x = 0.0f;   trucXoay.y = 1.0f;   trucXoay.z = 0.0f;

   
   float gocXoay = soHoatHinh*0.2f;
   
   unsigned short chiSo = 0;
   while( chiSo < soLuongBatDien ) {
      // ---- xài vị trí đầu cho bong bóng di chuyển hơi khác nhau
      Vecto viTriDau = danhSachBatDien[chiSo].viTriDau;
      Quaternion xoay = datQuaternionTuVectoVaGocQuay( &trucXoay, gocXoay );
      datBienHoaChoVat( &(danhSachBatDien[chiSo]), &phongTo, &xoay, &viTriDau );
      
      chiSo++;
   }
}

void nangCapCotNhay( VatThe *danhSachBatDien, unsigned short soLuongBatDien, unsigned short soHoatHinh ) {
   
   Vecto phongTo;
   phongTo.x = 1.0f;   phongTo.y = 1.0f;   phongTo.z = 1.0f;
   
   Vecto trucXoay;
   Quaternion xoay;
   xoay.w = 1.0f;    xoay.x = 0.0f;     xoay.y = 0.0f;     xoay.z = 0.0f;
   
   // ---- mỗi cột có ba thành phần
   unsigned short chiSo = 0;
   while( chiSo < soLuongBatDien ) {
      // ---- xài vị trí đầu cho bong bóng di chuyển hơi khác nhau
      Vecto viTriDau = danhSachBatDien[chiSo].viTriDau;
      viTriDau.y += 0.5f*sinf( (soHoatHinh + chiSo)*0.3f  );
      datBienHoaChoVat( &(danhSachBatDien[chiSo]), &phongTo, &xoay, &viTriDau );
      viTriDau.y += 0.5f;
      datBienHoaChoVat( &(danhSachBatDien[chiSo+1]), &phongTo, &xoay, &viTriDau );
      viTriDau.y += 0.5f;
      datBienHoaChoVat( &(danhSachBatDien[chiSo+2]), &phongTo, &xoay, &viTriDau );
      chiSo +=3;
   }
}

// ---- xài vị trí sao gai cho cách trái banh né sao gai 
void moPhongTraiBanhGiaoThong( VatThe *danhSachVatThe, unsigned short soTraiBanhDau, unsigned short soTraiBanhCuoi, Vecto *viTriSaoGai ) {

   VatThe *traiBanh;
   Vecto phongTo;
   phongTo.x = 1.0f;
   phongTo.y = 1.0f;
   phongTo.z = 1.0f;

   // ----
   unsigned short soTraiBanh = soTraiBanhDau;
   while ( soTraiBanh < soTraiBanhCuoi ) {
      //         printf( "%d  1.0\n", soTraiBanh );
      traiBanh = &(danhSachVatThe[soTraiBanh]);
      // ---- vị trí bây giờ
      float *dich = traiBanh->dich;
      Vecto viTriTraiBanh;
      viTriTraiBanh.x = dich[12];
      viTriTraiBanh.y = dich[13];
      viTriTraiBanh.z = dich[14];

      // ---- tính vị trí tơưng đối với sao gai
      Vecto huongDenSaoGai;
      huongDenSaoGai.x = viTriSaoGai->x - viTriTraiBanh.x;
//      huongDenSaoGai.y = viTriSaoGai.y - viTriTraiBanh.y;
      huongDenSaoGai.z = viTriSaoGai->z - viTriTraiBanh.z;
      // ---- không cần toạ độ y vì mô phỏng này chí đi trên đường băng thẳng
      Vecto vanTocTraiBanh;
      Vecto trucXoay;  // cho xoay trái banh đúng hướng
      float cachBinh = huongDenSaoGai.x*huongDenSaoGai.x + huongDenSaoGai.z*huongDenSaoGai.z;
//      printf( "soLuongTraiBanh %d  cachBinh %5.3f\n", soTraiBanh, cachBinh );
      if( cachBinh > 500.0 )  // không cần né sao gai
         vanTocTraiBanh = vanTocChoViTriChoPhimTruong0( &viTriTraiBanh, traiBanh->duongVao, traiBanh->duongRa, &(traiBanh->thaiTrang), &trucXoay );
      else {   // xem nếu cần né, viTriSaoGai->x > 0.0f vì chỉ né khi sao gai chuần bị ra và đã ra vòng
         if( (traiBanh->thaiTrang == kTHAI_TRANG__RA_VONG) && (traiBanh->duongRa == kDUONG_Z) && (viTriSaoGai->x > 0.0f) )
            vanTocTraiBanh = vanTocNeSaoGaiRaDuongZ( &viTriTraiBanh, &(traiBanh->thaiTrang), &trucXoay, viTriSaoGai );
         else // đi bình thường
            vanTocTraiBanh = vanTocChoViTriChoPhimTruong0( &viTriTraiBanh, traiBanh->duongVao, traiBanh->duongRa, &(traiBanh->thaiTrang), &trucXoay );

      }

      traiBanh->vanToc.x = vanTocTraiBanh.x;
      // traiBanh->vanToc.y = vanTocTraiBanh.y; 
      traiBanh->vanToc.z = vanTocTraiBanh.z;
      viTriTraiBanh.x += traiBanh->vanToc.x;
      // viTriTraiBanh.y += traiBanh->vanToc.y;
      viTriTraiBanh.z += traiBanh->vanToc.z;
      //         if( soTraiBanh == 5 )
      //         printf( "  %d  %5.3f %5.3f   %5.3f %5.3f   ", soTraiBanh, viTriTraiBanh.x, viTriTraiBanh.z, vanTocMoi.x, vanTocMoi.z );
      // ---- nâng cấp biến hóa dịch
      datDich( traiBanh->dich, viTriTraiBanh.x, viTriTraiBanh.y, viTriTraiBanh.z );
      datDich( traiBanh->nghichDich, -viTriTraiBanh.x, -viTriTraiBanh.y, -viTriTraiBanh.z );
      // ---- tính góc xoay quanh trục
      Quaternion xoay = tinhXoayChoVatThe( traiBanh, trucXoay, kTOC_DO_TRAI_BANH_PHIM_TRUONG0, traiBanh->hinhDang.hinhCau.banKinh );
      datBienHoaChoVat( traiBanh, &phongTo, &xoay, &viTriTraiBanh );

      soTraiBanh++;
   }

}


Quaternion tinhXoayChoVatThe( VatThe *vatThe, Vecto trucXoay, float tocDo, float banKinh ) {

   float gocXoay = tocDo/banKinh;
   Quaternion quaternionXoay = datQuaternionTuVectoVaGocQuay( &trucXoay, gocXoay );
   Quaternion quaternionVatThe = vatThe->quaternion;
   Quaternion xoayMoi = nhanQuaternionVoiQuaternion( &quaternionVatThe, &quaternionXoay );
   return xoayMoi;
}



Vecto vanTocChoViTriChoPhimTruong0( Vecto *viTri, unsigned char duongVao, unsigned char duongRa, unsigned char *thaiTrang, Vecto *trucXoay ) {
   
   Vecto vanToc;
   vanToc.x = 0.0f;
   vanToc.z = 0.0f;

   if( *thaiTrang == kTHAI_TRANG__VAO_VONG ) {
      if( duongVao == kDUONG_X ) {
         vanToc.x = kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         trucXoay->x = 0.0f;
         trucXoay->y = 0.0f;
         trucXoay->z = -1.0f;
      }
      else {
         vanToc.z = kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         trucXoay->x = 1.0f;
         trucXoay->y = 0.0f;
         trucXoay->z = 0.0f;
      }

      // ---- xem trong vào vòng tròn chưa
      float banKinhBinh = viTri->x*viTri->x + viTri->z*viTri->z;
      
      if( banKinhBinh < kBAN_KINH__VONG_NOI_BINH ) {
         if( duongVao == duongRa )   // đi vào sâu hơn
            *thaiTrang = kTHAI_TRANG__TRONG_VONG;
      }
      else if( banKinhBinh < kBAN_KINH__VONG_NGOAI_BINH ) {
         if( duongVao != duongRa ) // không đi vào sâu
            *thaiTrang = kTHAI_TRANG__TRONG_VONG;
      }
   }
   else if( *thaiTrang == kTHAI_TRANG__TRONG_VONG ) {
      float goc = atanf( viTri->z/viTri->x );
      if( viTri->x < 0.0f ) {
         if( viTri->z < 0.0f )
            goc -= 3.1415926f;
         else
            goc += 3.1415926f;
      }
      
      float banKinhBinh = viTri->x*viTri->x + viTri->z*viTri->z;
      float cosGoc = cosf( goc );
      float sinGoc = sinf( goc );
      vanToc.x = kTOC_DO_TRAI_BANH_PHIM_TRUONG0*sinGoc;
      vanToc.z = -kTOC_DO_TRAI_BANH_PHIM_TRUONG0*cosGoc;
      
      trucXoay->x = -cosGoc;
      trucXoay->y = 0.0f;
      trucXoay->z = -sinGoc;
      
      // ---- xem đến lối ra chưa
      if( duongRa == kDUONG_X ) {
         unsigned char trenDuongX = kDUNG;
         if( viTri->z < -3.0f )
            trenDuongX = kSAI;
         else if( viTri->z > 0.0f )
            trenDuongX = kSAI;
         else if( viTri->x > 0.0f )
            trenDuongX = kSAI;

         if( trenDuongX ) {
            *thaiTrang = kTHAI_TRANG__RA_VONG;
            viTri->z = -3.0f;
         }
      }
      else {
         unsigned char trenDuongZ = kDUNG;
         if( viTri->x < 0.0f )
            trenDuongZ = kSAI;
         else if( viTri->x > 3.0f )
            trenDuongZ = kSAI;
         else if( viTri->z > 0.0f )
            trenDuongZ = kSAI;
         
         if( trenDuongZ ) {
            *thaiTrang = kTHAI_TRANG__RA_VONG;
            viTri->x = 3.0f;
         }
      }
   }
   else if( *thaiTrang == kTHAI_TRANG__RA_VONG ) {
      if( duongRa == kDUONG_X ) {
         vanToc.x = -kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         trucXoay->x = 0.0f;
         trucXoay->y = 0.0f;
         trucXoay->z = 1.0f;
         
         // ---- nếu xa, đi vào nữa; NẾU MUỐN XÀI NÀY PHẢI ĐƯỢC ĐỔI BIẾN ĐƯỜNG VÀO 
/*         if( viTri->x < -500.0f ) {
            viTri->z = 3.0f;
            *thaiTrang = kTHAI_TRANG__VAO_VONG;
         } */
 
      }
      else {
         vanToc.z = -kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         trucXoay->x = -1.0f;
         trucXoay->y = 0.0f;
         trucXoay->z = 0.0f;
         // ---- nếu xa, đi vào nữa; NẾU MUỐN XÀI NÀY PHẢI ĐƯỢC ĐỔI BIẾN ĐƯỜNG VÀO 
/*         if( viTri->z < -500.0f ) {
            viTri->x = -3.0f;
            *thaiTrang = kTHAI_TRANG__VAO_VONG;
         } */
      }

   }
   
   // ---- đường hướng z 
   unsigned char trenDuongZ = kDUNG;
   if( viTri->x < -5.0f )
      trenDuongZ = kSAI;
   else if( viTri->x > 0.0f )
      trenDuongZ = kSAI;
   else if( viTri->z > -12.0f )
      trenDuongZ = kSAI;
   
   if( trenDuongZ ) {
      vanToc.z = kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
//      printf( "---> +z " );
      return vanToc;
   }
   
   // ---- không vận tốc
   return vanToc;
}

#define kCACH_XA_TU_GIUA_DUONG 8.0f


Vecto vanTocNeSaoGaiRaDuongZ( Vecto *viTriTraiBanh, unsigned char *thaiTrang, Vecto *trucXoay, Vecto *viTriSaoGai ) {
   
   Vecto vanTocTraiBanh;
   vanTocTraiBanh.x = 0.0f;   vanTocTraiBanh.y = 0.0f;    vanTocTraiBanh.z = 0.0f;
   // ---- tính quãng tương đối với trái banh
   float quangTuongDoi = viTriSaoGai->z - viTriTraiBanh->z;

   if( (quangTuongDoi < 12.0f) && (quangTuongDoi > -2.0f) ) {  // sao gai ở đang sau
      if( viTriTraiBanh->x < kCACH_XA_TU_GIUA_DUONG ) {  // đi ra ngoài đường
         vanTocTraiBanh.x = 1.3f*kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         vanTocTraiBanh.z = -1.3f*kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         trucXoay->x = -0.707f;
         trucXoay->y = 0.0f;
         trucXoay->z = -0.707f;
      }
      else {  // hạy songๆ với đường mà chậm lại một chút
         vanTocTraiBanh.x = 0.0f;
         vanTocTraiBanh.z = -0.9f*kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         trucXoay->x = -1.0f;
         trucXoay->y = 0.0f;
         trucXoay->z = 0.0f;
      }
   }
   else if( (quangTuongDoi > -15.0f) && (quangTuongDoi < -2.0f) ) { // sao gai ở phía trước

      if( viTriTraiBanh->x > 3.0f ) { // trở lại lằn trên đường
         vanTocTraiBanh.x = -1.2f*kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         vanTocTraiBanh.z = -kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         trucXoay->x = -1.0f;
         trucXoay->y = 0.0f;
         trucXoay->z = 1.2f;
      }
      else {  // tham gia giao thông bình thường
         viTriTraiBanh->x = 3.0f;  // đặt lại trái vào lằn như trước
         vanTocTraiBanh.x = 0.0f;  // không còn vận tốc hướng x
         vanTocTraiBanh.z = -kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
         trucXoay->x = -1.0f;
         trucXoay->y = 0.0f;
         trucXoay->z = 0.0f;
      }
   }
   else {   // không cần né
      vanTocTraiBanh.x = 0.0f;
      vanTocTraiBanh.z = -kTOC_DO_TRAI_BANH_PHIM_TRUONG0;
      trucXoay->x = -1.0f;
      trucXoay->y = 0.0f;
      trucXoay->z = 0.0f;
   }
//   if( quangTuongDoi < 400.0 )
//      printf( "quangTuongDoi %5.3f  vanTocTraiBanh %5.3f %5.3f\n", quangTuongDoi, vanTocTraiBanh.x, vanTocTraiBanh.z );
   return vanTocTraiBanh;
}

#pragma mark ---- PHIM TRƯỜNG 1
void chuanBiMayQuayPhimVaMatTroiPhimTruong1( PhimTruong *phimTruong );
unsigned short datMatDat( VatThe *danhSachVatThe );
unsigned short datCacDoiHinhNonCat( VatThe *danhSachVatThe, float mucDat );
unsigned short datDayNuiParabol( VatThe *danhSachVat, float mucDat );
unsigned short datSongBongBong( VatThe *danhSachVatThe );

unsigned short datQuocLo( VatThe *danhSachVatThe );
unsigned short datCauSongBongBong( VatThe *danhSachVat );
unsigned short datLapPhuong( VatThe *danhSachVat );
unsigned short datDayHinhTru( VatThe *danhSachVat );
unsigned short datNapDayHinhTru( VatThe *danhSachVat );
unsigned short datHinhCauKhongLoXoay( VatThe *danhSachVat );
unsigned short datDayHinhNonHuongZ_PT_1( VatThe *danhSachVat );
unsigned short datHemVucNui( VatThe *danhSachVat );
unsigned short datTuongSong( Vecto viTri, float dichLen, float banKinh, VatThe *danhSachVat );
unsigned short datBayBongBong( VatThe *danhSachVat );

unsigned short datCong( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datCotCong( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datMaiDinhCong( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datHaiKimTuThapXoay( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datHaiThauKinhXoay( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datVuDinhCong( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datChanLaCo( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datLaCo( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datDiaSauLaCo( VatThe *danhSachVat, Vecto viTriDay );
unsigned short datLocXoay( VatThe *danhSachVat, Vecto viTriDay );

void nangCapPhimTruong1_mayQuayPhim( PhimTruong *phimTruong );
void nangCapPhimTruong1_nhanVat( PhimTruong *phimTruong, unsigned short soHoatHinh );
void nangCapVaiChanh_PT_1( VatThe *vaiChanh, unsigned short soHoatHinh );
void nangCapSaoGai_PT_1( VatThe *saoGai, unsigned short soHoatHinh );
void nangCapHinhCauKhongLoXoay( VatThe *hinhCauKhongLoXoay );
void nangCapNapDayHinhTru( VatThe *mangVatThe, unsigned short soLuongVatThe );
void nangCapBayBongBong( VatThe *danhSachBongBong, unsigned short soLuongBongBong, unsigned short soHoatHinh );
void nangCapHaiKimTuThapXoay( VatThe *kimTuThap );
void nangCapHaiThauKinhXoay( VatThe *kimTuThap );
void nangCapVuDinhCong( VatThe *danhSachVat, unsigned short soHoatHinh );
void nangCapLaCo( VatThe *danhSachVat, unsigned short soLuongVatThe, unsigned short soHoatHinh );
void nangCapLocXoay( VatThe *danhSachVat, unsigned short soHoatHinh );

#define kNHAN_VAT__HINH_CAU_KHONG_LO_XOAY 3
#define kNHAN_VAT__HAI_KIM_TU_THAP_XOAY   4
#define kNHAN_VAT__HAI_THAU_KINH_XOAY     5
#define kNHAN_VAT__VU_DINH_CONG           6
#define kNHAN_VAT__LOC_XOAY_0             7
#define kNHAN_VAT__LOC_XOAY_1             8
#define kNHAN_VAT__LOC_XOAY_2             9
#define kNHAN_VAT__NAP_DAY_HINH_TRU_DAU  10
#define kNHAN_VAT__NAP_DAY_HINH_TRU_CUOI 11
#define kNHAN_VAT__MIEN_LA_CO_DAU        12
#define kNHAN_VAT__MIEN_LA_CO_CUOI       13
#define kNHAN_VAT__BONG_BONG_DAU_PT1     14
#define kNHAN_VAT__BONG_BONG_CUOI_PT1    15

#define kSO_LUONG__BONG_BONG_TRONG_BAY  100  // <----- 5000
#define kTOC_DO_BONG_BONG_TRONG_BAY     0.004f       // tốc độ bong bóng trong bay
#define kSO_LUONG__HAT_BAY_TRONG_LOC_XOAY  300   // số lượng hạt bay trong lốc xoay

PhimTruong datPhimTruongSo1( unsigned int argc, char **argv ) {
   
   PhimTruong phimTruong;
   phimTruong.soNhoiToiDa = 5;
   
   unsigned int soHoatHinhDau = 0;
   unsigned int soHoatHinhCuoi = 2000;    // số bức ảnh cuối cho phim trường này
   
   docThamSoHoatHinh( argc, argv, &soHoatHinhDau, &soHoatHinhCuoi );
   if( soHoatHinhDau > 1999 )
      soHoatHinhDau = 1999;
   if( soHoatHinhCuoi > 2000 )    // số bức ảnh cuối cho phim trường này
      soHoatHinhCuoi = 2000;
   
   phimTruong.soHoatHinhDau = soHoatHinhDau;
   phimTruong.soHoatHinhHienTai = soHoatHinhDau;
   phimTruong.soHoatHinhCuoi = soHoatHinhCuoi;

   phimTruong.danhSachVatThe = malloc( kSO_LUONG_VAT_THE_TOI_DA*sizeof(VatThe) );
   
   // ---- chuẩn bị máy quay phim
   chuanBiMayQuayPhimVaMatTroiPhimTruong1( &phimTruong );
   Mau mauDinhTroi;
   mauDinhTroi.d = 0.1f;   mauDinhTroi.l = 0.1f;   mauDinhTroi.x = 0.5f;   mauDinhTroi.dd = 1.0f;
   Mau mauChanTroi;  // chỉ có một;
   mauChanTroi.d = 0.9f;  mauChanTroi.l = 0.9f;   mauChanTroi.x = 1.0f;    mauChanTroi.dd = 1.0f;
   phimTruong.hoaTietBauTroi = datHoaTietBauTroi( &mauDinhTroi, &mauChanTroi, &mauChanTroi, 0.0f );
   
   VatThe *danhSachVat = phimTruong.danhSachVatThe;
   phimTruong.soLuongVatThe = 0;

   // ---- đất
   phimTruong.soLuongVatThe += datMatDat( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.soLuongVatThe += datCacDoiHinhNonCat( &(danhSachVat[phimTruong.soLuongVatThe]), 100.0f );
   phimTruong.soLuongVatThe += datDayNuiParabol( &(danhSachVat[phimTruong.soLuongVatThe]), 100.0f );
   // ---- đường
   phimTruong.soLuongVatThe += datQuocLo( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- cầu
   phimTruong.soLuongVatThe += datCauSongBongBong( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- lập phương
   phimTruong.soLuongVatThe += datLapPhuong( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- dãy hình trụ
   phimTruong.soLuongVatThe += datDayHinhTru( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- dãy hình gần lề đường lộ
//   phimTruong.soLuongVatThe += datDayHinhNonHuongZ_PT_1( &(danhSachVat[phimTruong.soLuongVatThe]) );
   
   phimTruong.soLuongVatThe += datSongBongBong( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- hai tường hẻm núi
   phimTruong.soLuongVatThe += datHemVucNui( &(danhSachVat[phimTruong.soLuongVatThe]) );
   
   // ---- cổng của cầu
   Vecto viTriDay;
   viTriDay.x = 0.0f;   viTriDay.y = 100.0f;   viTriDay.z = 190.0f;
   phimTruong.soLuongVatThe += datCong( &(danhSachVat[phimTruong.soLuongVatThe]), viTriDay );

   // ---- CÁC NHÂN VẬT
   // sao gai
   // Cái sẽ bị hư gì hệ thống tăng tốc độ kết xuất sẽ đổi thứ tự vật thề <---------
   Vecto viTri;
   viTri.x = 3.0f;   viTri.y = 100.0f + kSAO_GAI__BAN_KINH*1.4f;     viTri.z = 1420.0f;
   phimTruong.nhanVat[kNHAN_VAT__SAO_GAI] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datKeThuSaoGai( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );
   // ---- vai chánh
   viTri.x = 3.0f;   viTri.y = 100.0f + 0.7f;     viTri.z = 1400.0f;  // 0.7 là bán kính vai chánh
   phimTruong.nhanVat[kNHAN_VAT__VAI_CHANH] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datVaiChanh( &(danhSachVat[phimTruong.soLuongVatThe]) , viTri );
   // ---- hình cầu khổng lộ xoay
   phimTruong.nhanVat[kNHAN_VAT__HINH_CAU_KHONG_LO_XOAY] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datHinhCauKhongLoXoay( &(danhSachVat[phimTruong.soLuongVatThe]) );
   // ---- hai kim tư tháp xoay
   phimTruong.nhanVat[kNHAN_VAT__HAI_KIM_TU_THAP_XOAY] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datHaiKimTuThapXoay( &(danhSachVat[phimTruong.soLuongVatThe]), viTriDay );
   // ---- hai thấu kính xoay
   phimTruong.nhanVat[kNHAN_VAT__HAI_THAU_KINH_XOAY] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datHaiThauKinhXoay( &(danhSachVat[phimTruong.soLuongVatThe]), viTriDay );
   // ---- vù đỉnh cổng
   phimTruong.nhanVat[kNHAN_VAT__VU_DINH_CONG] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datVuDinhCong( &(danhSachVat[phimTruong.soLuongVatThe]), viTriDay );

   // ---- các nắp cho dãy hình trụ
   phimTruong.nhanVat[kNHAN_VAT__NAP_DAY_HINH_TRU_DAU] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datNapDayHinhTru( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.nhanVat[kNHAN_VAT__NAP_DAY_HINH_TRU_CUOI] = phimTruong.soLuongVatThe;

   // ---- sàn lá cờ
   viTri.x = -18.0f;   viTri.y = 100.0f;     viTri.z = -210.0f;
   phimTruong.soLuongVatThe += datChanLaCo( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );
   viTri.x = 18.0f;
   phimTruong.soLuongVatThe += datChanLaCo( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );
   
   // ---- dĩa sau lá cờ
   viTri.x = -18.0f;   viTri.y = 100.0f;     viTri.z = -222.0f;
   phimTruong.soLuongVatThe += datDiaSauLaCo( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );
   viTri.x = 18.0f;
   phimTruong.soLuongVatThe += datDiaSauLaCo( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );

   // ---- hai lá cờ
   viTri.x = -18.0f;   viTri.y = 103.0f;     viTri.z = -210.0f;
   phimTruong.nhanVat[kNHAN_VAT__MIEN_LA_CO_DAU] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datLaCo( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );
   viTri.x = 18.0f;
   phimTruong.soLuongVatThe += datLaCo( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );
   phimTruong.nhanVat[kNHAN_VAT__MIEN_LA_CO_CUOI] = phimTruong.soLuongVatThe;
 
   // ----- các lốc xoay
   viTri.x = -1770.0f;   viTri.y = 101.0f;     viTri.z = -720.0f;
   phimTruong.nhanVat[kNHAN_VAT__LOC_XOAY_0] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datLocXoay( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );

   viTri.x = -3845.0f;   viTri.y = 101.0f;     viTri.z = -655.0f;
   phimTruong.nhanVat[kNHAN_VAT__LOC_XOAY_1] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datLocXoay( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );

   viTri.x = -3500.0f;   viTri.y = 101.0f;     viTri.z = -800.0f;
   phimTruong.nhanVat[kNHAN_VAT__LOC_XOAY_2] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datLocXoay( &(danhSachVat[phimTruong.soLuongVatThe]), viTri );

   // ---- các bong bóng bay trong hẻm núi
   phimTruong.nhanVat[kNHAN_VAT__BONG_BONG_DAU_PT1] = phimTruong.soLuongVatThe;
   phimTruong.soLuongVatThe += datBayBongBong( &(danhSachVat[phimTruong.soLuongVatThe]) );
   phimTruong.nhanVat[kNHAN_VAT__BONG_BONG_CUOI_PT1] = phimTruong.soLuongVatThe;

   // ---- nâng cấp mô phỏng cho các trái banh và nhân vật
   unsigned short soHoatHinh = 0;
   while ( soHoatHinh < phimTruong.soHoatHinhDau ) {
      nangCapPhimTruong1_nhanVat( &phimTruong, soHoatHinh );
      soHoatHinh++;
   }

   return phimTruong;
}

void chuanBiMayQuayPhimVaMatTroiPhimTruong1( PhimTruong *phimTruong ) {
   // ==== máy quay phim
   phimTruong->mayQuayPhim.kieuChieu = kKIEU_CHIEU__PHOI_CANH;
   // ---- vị trí bắt đầu cho máy quay phim
   phimTruong->mayQuayPhim.viTri.x = 400.0f;
   phimTruong->mayQuayPhim.viTri.y = 200.0f;
   phimTruong->mayQuayPhim.viTri.z = 800.0f;
   phimTruong->mayQuayPhim.cachManChieu = 5.0f;
   Vecto trucQuayMayQuayPhim;
   trucQuayMayQuayPhim.x = 0.0f;
   trucQuayMayQuayPhim.y = 1.0f;
   trucQuayMayQuayPhim.z = 0.0f;
   phimTruong->mayQuayPhim.quaternion = datQuaternionTuVectoVaGocQuay( &trucQuayMayQuayPhim, 4.71f );
   quaternionQuaMaTran( &(phimTruong->mayQuayPhim.quaternion), phimTruong->mayQuayPhim.xoay );
   //   mayQuayPhim.diChuyen = ();

   // ---- mặt trời
   Vecto anhSangMatTroi;
   anhSangMatTroi.x = -1.0f;
   anhSangMatTroi.y = -0.5f;
   anhSangMatTroi.z = -0.5f;
   donViHoa( &anhSangMatTroi );
   phimTruong->matTroi.huongAnh = anhSangMatTroi;
   phimTruong->matTroi.mauAnh.d = 1.0f;
   phimTruong->matTroi.mauAnh.l = 1.0f;
   phimTruong->matTroi.mauAnh.x = 1.0f;
   phimTruong->matTroi.mauAnh.dd = 1.0f;
}


unsigned short datMatDat( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ==== bờ đất
   Mau mauDat0;
   mauDat0.d = 1.0f;   mauDat0.l = 0.55f;   mauDat0.x = 0.0f;   mauDat0.dd = 1.0f;   mauDat0.p = 0.0f;
   Mau mauDat1;
   mauDat1.d = 1.0f;   mauDat1.l = 1.0f;   mauDat1.x = 1.0f;   mauDat1.dd = 1.0f;   mauDat1.p = 0.0f;
   
   // ---- mặt đất bên +z, mặt trên cao 100,0
   viTri.x = 0.0f;      viTri.y = 50.0f;        viTri.z = 2000.0f;
   danhSachVat[0].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[0].soHoaTiet = kHOA_TIET__CA_RO;

   // ----
   mauDat0.d = 1.0f;   mauDat0.l = 0.6f;   mauDat0.x = 0.2f;   mauDat0.dd = 1.0f;
   viTri.z = 1600.0f;
   danhSachVat[1].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[1].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.z = 1200.0f;
   danhSachVat[2].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[2].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[2].soHoaTiet = kHOA_TIET__CA_RO;
   
   mauDat0.d = 1.0f;   mauDat0.l = 0.8f;   mauDat0.x = 0.1f;   mauDat0.dd = 1.0f;  // đỏ hơn một chút
   viTri.z = 800.0f;
   danhSachVat[3].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[3].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternion, &viTri );
   danhSachVat[3].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[3].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.z = 400.0f;
   danhSachVat[4].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[4].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternion, &viTri );
   danhSachVat[4].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[4].soHoaTiet = kHOA_TIET__CA_RO;
   
   // ---- mặt đất bên -z, mặt trên cao 100,0
   viTri.x = 475.0f;  // ngắn hơn bên thường
   viTri.z = -600.0f;
//   danhSachVat[5].hinhDang.hop = datHop( 1549.999f, 99.999f, 399.999f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_VAT_THE__BOOL;
   danhSachVat[5].mucDichBool = 1;
   danhSachVat[5].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].hoaTiet.hoaTietCaRoMin = datHoaTietCaRoMin( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[5].soHoaTiet = kHOA_TIET__CA_RO_MIN;
   
   danhSachVat[5].soLuongVatThe = 3;
   danhSachVat[5].danhSachVatThe = malloc( sizeof( VatThe )*3 );
   
   viTri.x = 0.0f;   viTri.y = 0.0f;  viTri.z = 0.0f;
   danhSachVat[5].danhSachVatThe[0].hinhDang.hop = datHop( 1559.999f, 99.999f, 399.999f, &(danhSachVat[5].danhSachVatThe[0].baoBiVT) );
   danhSachVat[5].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[5].danhSachVatThe[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5].danhSachVatThe[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].danhSachVatThe[0].giaTri = 1;

   // ---- hình nón lớn
   viTri.x = -580.0f;   viTri.y = 2.0f;  viTri.z = 0.0f;
   danhSachVat[5].danhSachVatThe[1].hinhDang.hinhNon = datHinhNon( 0.0f, 70.0f, 100.0f, &(danhSachVat[5].danhSachVatThe[1].baoBiVT) );
   danhSachVat[5].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[5].danhSachVatThe[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5].danhSachVatThe[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].danhSachVatThe[1].giaTri = -1;

   // ---- hình nón lớn
   viTri.x = -400.0f;   viTri.y = 2.0f;  viTri.z = -100.0f;
   danhSachVat[5].danhSachVatThe[2].hinhDang.hinhNon = datHinhNon( 0.0f, 70.0f, 100.0f, &(danhSachVat[5].danhSachVatThe[2].baoBiVT) );
   danhSachVat[5].danhSachVatThe[2].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[5].danhSachVatThe[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5].danhSachVatThe[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].danhSachVatThe[2].giaTri = -1;

   // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
   tinhBaoBiVTChoVatTheGhep( &(danhSachVat[5]) );

   viTri.x = 0.0f;
   viTri.y = 50.0f;
   viTri.z = -1000.0f;
   danhSachVat[6].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[6].baoBiVT) );
   danhSachVat[6].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[6].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6]), &phongTo, &quaternion, &viTri );
   danhSachVat[6].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[6].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.z = -1400.0f;
   danhSachVat[7].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[7].baoBiVT) );
   danhSachVat[7].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[7].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[7]), &phongTo, &quaternion, &viTri );
   danhSachVat[7].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[7].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.z = -1800.0f;
   danhSachVat[8].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[8].baoBiVT) );
   danhSachVat[8].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[8].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[8]), &phongTo, &quaternion, &viTri );
   danhSachVat[8].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[8].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.z = -2200.0f;
   danhSachVat[9].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[9].baoBiVT) );
   danhSachVat[9].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[9].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[9]), &phongTo, &quaternion, &viTri );
   danhSachVat[9].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[9].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.z = -2600.0f;
   danhSachVat[10].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[10].baoBiVT) );
   danhSachVat[10].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[10].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[10]), &phongTo, &quaternion, &viTri );
   danhSachVat[10].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[10].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.z = -3000.0f;
   danhSachVat[11].hinhDang.hop = datHop( 2499.999f, 99.999f, 399.999f, &(danhSachVat[11].baoBiVT) );
   danhSachVat[11].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[11].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[11]), &phongTo, &quaternion, &viTri );
   danhSachVat[11].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat0, &mauDat1, 40.0f, 10.0f, 40.0f );
   danhSachVat[11].soHoaTiet = kHOA_TIET__CA_RO;

   return 12;
}

unsigned short datCacDoiHinhNonCat( VatThe *danhSachVat, float mucDat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   float banKinh = 0.0f;
   unsigned short soLuongVatThe = 0;
   Mau mau0;
   Mau mau1;

   // ==== bên +z
   // ---- -x
   mau0.d = 1.0f;   mau0.l = 0.6f;  mau0.x = 0.6f;   mau0.dd = 1.0f;   mau0.p = 0.0f;
   mau1.d = 1.0f;   mau1.l = 1.0f;  mau1.x = 1.0f;   mau1.dd = 1.0f;   mau1.p = 0.0f;

   viTri.x = -100.0f;
   viTri.y = mucDat + 9.0f;
   viTri.z = 430.0f;
   banKinh = 30.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 14.0f, banKinh + 0.0f, 20.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = -70.0f;
   viTri.y = mucDat + 5.0f;
   viTri.z = 310.0f;
   banKinh = 50.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = -270.0f;
   viTri.y = mucDat + 3.0f;
   viTri.z = 350.0f;
   banKinh = 40.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = -275.0f;
   viTri.y = mucDat + 8.0f;
   viTri.z = 358.0f;
   banKinh = 15.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = -550.0f;
   viTri.y = mucDat + 5.0f;
   viTri.z = 380.0f;
   banKinh = 105.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   // ----- +x
   viTri.x = 100.0f;
   viTri.y = mucDat + 5.0f;
   viTri.z = 550.0f;
   banKinh = 20.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = 220.0f;
   viTri.y = mucDat + 3.0f;
   viTri.z = 420.0f;
   banKinh = 60.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = 530.0f;
   viTri.y = mucDat + 3.0f;
   viTri.z = 380.0f;
   banKinh = 20.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = 450.0f;
   viTri.y = mucDat + 5.0f;
   viTri.z = 680.0f;
   banKinh = 100.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = 600.0f;
   viTri.y = mucDat + 2.0f;
   viTri.z = 880.0f;
   banKinh = 150.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   // ==== bên -z
   // -x
   viTri.x = -860.0f;
   viTri.y = mucDat - 4.0f;
   viTri.z = -820.0f;
   banKinh = 350.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = -400.0f;
   viTri.y = mucDat + 8.0f;
   viTri.z = -850.0f;
   banKinh = 180.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 14.0f, banKinh + 0.0f, 20.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = -80.0f;
   viTri.y = mucDat + 10.0f;
   viTri.z = -950.0f;
   banKinh = 50.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 21.0f, banKinh + 0.0f, 30.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   
   viTri.x = -50.0f;
   viTri.y = mucDat + -3.0f;
   viTri.z = -300.0f;
   banKinh = 25.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = -70.0f;
   viTri.y = mucDat + 2.0f;
   viTri.z = -330.0f;
   banKinh = 15.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   
   // ---- +x
   viTri.x = 35.0f;
   viTri.y = mucDat;
   viTri.z = -270.0f;
   banKinh = 20.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 3.5f, banKinh + 0.0f, 5.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = 850.0f;
   viTri.y = mucDat + 5.0f;
   viTri.z = -480.0f;
   banKinh = 80.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 14.0f, banKinh + 0.0f, 20.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = 150.0f;
   viTri.y = mucDat + 5.0f;
   viTri.z = -1050.0f;
   banKinh = 80.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 14.0f, banKinh + 0.0f, 20.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = -450.0f;
   viTri.y = mucDat + 13.0f;
   viTri.z = -800.0f;
   banKinh = 80.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 21.0f, banKinh + 0.0f, 30.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = 750.0f;
   viTri.y = mucDat + 6.5f;
   viTri.z = -680.0f;
   banKinh = 30.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 14.0f, banKinh + 0.0f, 20.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;
   
   viTri.x = 650.0f;
   viTri.y = mucDat + 3.5f;
   viTri.z = -550.0f;
   banKinh = 30.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 14.0f, banKinh + 0.0f, 20.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = 250.0f;
   viTri.y = mucDat + 3.0f;
   viTri.z = -860.0f;
   banKinh = 55.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   viTri.x = 60.0f;
   viTri.y = mucDat + 10.0f;
   viTri.z = -860.0f;
   banKinh = 30.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 14.0f, banKinh + 0.0f, 20.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = 400.0f;
   viTri.y = mucDat + 8.0f;
   viTri.z = -1020.0f;
   banKinh = 80.0f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( banKinh + 14.0f, banKinh + 0.0f, 20.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO;
   soLuongVatThe++;

   return soLuongVatThe;
}

unsigned short datDayNuiParabol( VatThe *danhSachVat, float mucDat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   unsigned short soLuongVatThe = 0;
   Mau mau0;
   Mau mau1;
   
   // bên -x
   mau0.d = 0.1f;   mau0.l = 0.0f;  mau0.x = 0.4f;   mau0.dd = 1.0f;   mau0.p = 0.0f;
   mau1.d = 1.0f;   mau1.l = 1.0f;  mau1.x = 1.0f;   mau1.dd = 1.0f;   mau1.p = 0.0f;
   
   float mangBanKinh[] = { -700.0f, -560.0f, -350.0f,  -300.0f, -350.0f,
      -480.0f, -450.0f, -400.0f,  -350.0f, -500.0f,
      -800.0f, -430.0f, -600.0f, -800.0f, -800.0f,
      -500.0f, -500.0f, -1200.0f, -750.0f};
   
   float mangBeCao[] =   { -1100.0f, 580.0,  450.0f,   200.0f, 400.0f,
      280.0f,  800.0f, 400.0f,   250.0f,  500.0f,
      500.0f, 300.0f, 400.0f, 300.0f, 500.0f,
      200.0f, 300.0f, 1000.0f, 550.0f};
   
   float mangViTri_x[] = { -1200.0f, -1000.0f, -500.0f, -1000.0f, -2000.0f,
      -1600.0f, -2500.0f, -300.0f, -2200.0f, -3500.0f,
      -3500.0f, -4400.0f, 350.0f, 800.0f, 1200.0f,
      2200.0f, 1400.0f, 3400.0f, 3000.0f};
   
   float mangViTri_z[] = { -6000.0f, -5500.0f, -3200.0f, -3500.0f, -3600.0f,
      -3700.0f, -3800.0f, -4080.0f, -4200.0f, -5000.0f,
      -5500.0f, 3500.0f, -5075.0f, -4000.0f, -4500.0f,
      -5000.0f, -5200.0f, -5500.0f, -6500.0f, };


   while( soLuongVatThe < 19 ) {
      float beCao = mangBeCao[soLuongVatThe];
      viTri.x = mangViTri_x[soLuongVatThe];
      viTri.y = mucDat + mangBeCao[soLuongVatThe];
      viTri.z = mangViTri_z[soLuongVatThe];

      danhSachVat[soLuongVatThe].hinhDang.matParabol = datMatParabol( mangBanKinh[soLuongVatThe], mangBeCao[soLuongVatThe], &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__MAT_PARABOL;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietCaRoMin = datHoaTietCaRoMin( &mau0, &mau1, 100.0f, 100.0f, 100.0f );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__CA_RO_MIN;
      soLuongVatThe++;
   }

   return soLuongVatThe;
}


unsigned short datSongBongBong( VatThe *danhSachVat ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;

   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ---- mặt song
   Mau mauSong;
   mauSong.d = 0.5f;  mauSong.l = 0.5f;  mauSong.x = 0.5f;  mauSong.dd = 1.0f;   mauSong.p = 0.8f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = -170.0f;       viTri.z = -100.0f;
   danhSachVat[0].hinhDang.matSong = datMatSong( 2000.0f, 600.0f, 0.3f, 0.3f, 0.1f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__MAT_SONG;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauSong );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;

   return 1;
}

unsigned short datQuocLo( VatThe *danhSachVat ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ==== bờ đất
   Mau mauDat;
   mauDat.d = 1.0f;   mauDat.l = 0.8f;   mauDat.x = 0.4f;   mauDat.dd = 1.0f;   mauDat.p = 0.0f;
   
   float mangViTri_z[] = { 2400.0f, 2000.0f, 1600.0f, 1200.0f, 800.0f, 400.0f,
      -420.0f, -820.0f, -1220.0f, -1620.0f, -2020.0f, -2400.0f, -2800.0f};

   // ==== mặt đường
   Mau mauOc0;
   Mau mauOc1;
   Mau mauOc2;
   mauOc0.d = 0.7f;     mauOc0.l = 0.5f;      mauOc0.x = 0.3f;     mauOc0.dd = 1.0f;    mauOc0.p = 0.0f;
   mauOc1.d = 0.8f;     mauOc1.l = 0.6f;     mauOc1.x = 0.4f;     mauOc1.dd = 1.0f;    mauOc1.p = 0.0f;
   mauOc2.d = 0.85f;     mauOc2.l = 0.7f;      mauOc2.x = 0.4f;     mauOc2.dd = 1.0f;    mauOc2.p = 0.0f;
   
   viTri.x = 0.0f;      viTri.y = 100.1f;
   Vecto huongNgang;
   huongNgang.x = 1.0f;   huongNgang.y = 0.0f;   huongNgang.z = 0.0f;
   Vecto huongDoc;
   huongDoc.x = 0.0f;   huongDoc.y = 0.0f;   huongDoc.z = 1.0f;

   unsigned short soLuongVatThe = 0;
   while( soLuongVatThe < 13 ) {
      viTri.z = mangViTri_z[soLuongVatThe];
      danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 12.0f, 0.2f, 400.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mauDat, &mauOc0, &mauOc1, &mauOc2, 0.2f, 0.2f, 0.2f, 3.0f, 3.0f, 0.7f, 0.0f, 50.0f );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
      
      soLuongVatThe++;
   }

   // ==== LAN CAN

   Mau mauLanCan;
   mauLanCan.d = 1.0f;    mauLanCan.l = 0.4f;    mauLanCan.x = 0.0f;    mauLanCan.dd = 1.0f;    mauLanCan.p = 0.0f;
   mauOc0.d = 0.8f;     mauOc0.l = 0.3f;     mauOc0.x = 0.0f;     mauOc0.dd = 1.0f;    mauOc0.p = 0.0f;
   mauOc1.d = 0.75f;     mauOc1.l = 0.25f;     mauOc1.x = 0.0f;     mauOc1.dd = 1.0f;    mauOc1.p = 0.0f;
   mauOc2.d = 0.7f;     mauOc2.l = 0.2f;     mauOc2.x = 0.0f;     mauOc2.dd = 1.0f;    mauOc2.p = 0.0f;

   viTri.y = 100.7f;
   while( soLuongVatThe < 26 ) {
      viTri.z = mangViTri_z[soLuongVatThe-13];
      danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 0.5f, 1.0f, 400.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mauLanCan, &mauOc0, &mauOc1, &mauOc2, 0.1f, 0.1f, 0.1f, 25.0f, 10.0f, 0.2f, 0.0f, 20.0f );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
      
      soLuongVatThe++;
   }

   return soLuongVatThe;
}

unsigned short datCauSongBongBong( VatThe *danhSachVat ) {
   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ==== màu mặt đường
   Mau mauDat;
   mauDat.d = 0.5f;   mauDat.l = 0.2f;   mauDat.x = 0.6f;   mauDat.dd = 1.0f;   mauDat.p = 0.0f;
   Mau mau1;
   mau1.d = 1.0f;   mau1.l = 0.85f;   mau1.x = 1.0f;   mau1.dd = 1.0f;   mau1.p = 0.0f;

   // ---- mặt đường đi
   viTri.x = 0.0f;      viTri.y = 99.2f;        viTri.z = -10.0f;
   danhSachVat[0].hinhDang.hop = datHop( 15.0f, 2.0f, 420.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietSoc = datHoaTietSoc( &mauDat, &mau1, 20.0f, 0.5f, kTRUC_Z );
   danhSachVat[0].soHoaTiet = kHOA_TIET__SOC;
   
   // ---- lang cang -x
   mauDat.d = 0.7f;   mauDat.l = 0.35f;   mauDat.x = 0.7f;   mauDat.dd = 1.0f;
   mau1.d = 1.0f;   mau1.l = 0.85f;   mau1.x = 1.0f;   mau1.dd = 1.0f;
   viTri.y = 100.3f;
   viTri.x = -7.5f;
   danhSachVat[1].hinhDang.hop = datHop( 1.0f, 1.0f, 420.0f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietSoc = datHoaTietSoc( &mauDat, &mau1, 20.0f, 0.5f, kTRUC_Z );
   danhSachVat[1].soHoaTiet = kHOA_TIET__SOC;

   viTri.y = 99.0f;
   danhSachVat[2].hinhDang.hop = datHop( 1.0f, 1.0f, 420.0f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[2].hoaTiet.hoaTietSoc = datHoaTietSoc( &mauDat, &mau1, 20.0f, 0.5f, kTRUC_Z );
   danhSachVat[2].soHoaTiet = kHOA_TIET__SOC;

   // ---- lang cang +x
   viTri.y = 100.3f;
   viTri.x = 7.5f;
   danhSachVat[3].hinhDang.hop = datHop( 1.0f, 0.8f, 420.0f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[3].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternion, &viTri );
   danhSachVat[3].hoaTiet.hoaTietSoc = datHoaTietSoc( &mauDat, &mau1, 20.0f, 0.5f, kTRUC_Z );
   danhSachVat[3].soHoaTiet = kHOA_TIET__SOC;

   viTri.y = 99.0f;
   danhSachVat[4].hinhDang.hop = datHop( 1.0f, 1.0f, 420.0f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[4].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternion, &viTri );
   danhSachVat[4].hoaTiet.hoaTietSoc = datHoaTietSoc( &mauDat, &mau1, 20.0f, 0.5f, kTRUC_Z );
   danhSachVat[4].soHoaTiet = kHOA_TIET__SOC;

   // ---- rao chia lằn đường
   mauDat.d = 0.5f;   mauDat.l = 0.25f;   mauDat.x = 0.3f;
   mau1.d = 1.0f;   mau1.l = 0.8f;   mau1.x = 0.6f;
   viTri.x = 0.0f;    viTri.y = 100.7f;
   danhSachVat[5].hinhDang.hop = datHop( 0.5f, 1.0f, 420.0f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[5].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].hoaTiet.hoaTietSoc = datHoaTietSoc( &mauDat, &mau1, 20.0f, 0.5f, kTRUC_Z );
   danhSachVat[5].soHoaTiet = kHOA_TIET__SOC;

   return 6;
}

unsigned short datLapPhuong( VatThe *danhSachVat ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ==== màu mặt đường
   Mau mauDat;
   mauDat.d = 1.0f;   mauDat.l = 0.0f;   mauDat.x = 0.0f;   mauDat.dd = 1.0f;   mauDat.p = 0.0f;
   Mau mau1;
   mau1.d = 1.0f;   mau1.l = 0.95f;   mau1.x = 0.95f;   mau1.dd = 1.0f;   mau1.p = 0.0f;

   viTri.x = -800.0f;      viTri.y = 300.0f;        viTri.z = 1000.0f;
//   danhSachVat[0].hinhDang.hop = datHop( 15.0f, 0.5f, 400.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_VAT_THE__BOOL;
   danhSachVat[0].mucDichBool = 1;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[0].soHoaTiet = kHOA_TIET__CA_RO;

   danhSachVat[0].soLuongVatThe = 3;
   danhSachVat[0].danhSachVatThe = malloc( sizeof( VatThe )*3 );

   viTri.x = 0.0f;   viTri.y = 0.0f;  viTri.z = 0.0f;
   danhSachVat[0].danhSachVatThe[0].hinhDang.hop = datHop( 399.9f, 399.9f, 399.9f, &(danhSachVat[0].danhSachVatThe[0].baoBiVT) );
   danhSachVat[0].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[0].danhSachVatThe[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0].danhSachVatThe[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].danhSachVatThe[0].giaTri = 1;

   // ---- hình cầu lớn
   viTri.x = 100.0f;   viTri.y = 100.0f;  viTri.z = -100.0f;
   danhSachVat[0].danhSachVatThe[1].hinhDang.hinhCau = datHinhCau( 220.0f, &(danhSachVat[0].danhSachVatThe[1].baoBiVT) );
   danhSachVat[0].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[0].danhSachVatThe[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0].danhSachVatThe[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].danhSachVatThe[1].giaTri = -1;

   // ---- lổ hình cầu nhỏ
   viTri.x = 200.0f;   viTri.y = -120.0f;  viTri.z = 120.0f;
   danhSachVat[0].danhSachVatThe[2].hinhDang.hinhCau = datHinhCau( 50.0f, &(danhSachVat[0].danhSachVatThe[2].baoBiVT) );
   danhSachVat[0].danhSachVatThe[2].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[0].danhSachVatThe[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0].danhSachVatThe[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].danhSachVatThe[2].giaTri = -1;
   // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
   tinhBaoBiVTChoVatTheGhep( &(danhSachVat[0]) );

   // ---- cao nguyên ở đằng -z từ lập phương
   viTri.x = -900.0f;      viTri.y = 150.0f;        viTri.z = 400.0f;
   danhSachVat[1].hinhDang.hop = datHop( 699.9f, 99.0f, 499.9f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[1].soHoaTiet = kHOA_TIET__CA_RO;
   
   // ---- cao nguyên ở đằng -z từ lập phương
   viTri.x = -750.0f;      viTri.y = 140.0f;        viTri.z = 600.0f;
   danhSachVat[2].hinhDang.hop = datHop( 399.9f, 79.0f, 499.9f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[2].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[2].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = -750.0f;      viTri.y = 200.0f;        viTri.z = 1400.0f;
   danhSachVat[3].hinhDang.hop = datHop( 399.9f, 199.0f, 299.9f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[3].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternion, &viTri );
   danhSachVat[3].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[3].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.x = -500.0f;      viTri.y = 165.0f;        viTri.z = 1600.0f;
   danhSachVat[4].hinhDang.hop = datHop( 299.9f, 129.0f, 299.9f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[4].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternion, &viTri );
   danhSachVat[4].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[4].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = -680.0f;      viTri.y = 225.0f;        viTri.z = 2000.0f;
   danhSachVat[5].hinhDang.hop = datHop( 299.9f, 249.0f, 299.9f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[5].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[5].soHoaTiet = kHOA_TIET__CA_RO;

   // ---- cao nhuyên nhỏ thấp gần đường lộ
   mauDat.d = 1.0f;   mauDat.l = 0.8f;   mauDat.x = 0.6f;   mauDat.dd = 1.0f;
   mau1.d = 0.5f;   mau1.l = 0.25f;   mau1.x = 0.3f;   mau1.dd = 1.0f;
   viTri.x = -200.0f;      viTri.y = 115.0f;        viTri.z = 1000.0f;
//   danhSachVat[6].hinhDang.hop = datHop( 149.9f, 29.9f, 149.9f, &(danhSachVat[6].baoBiVT) );
   danhSachVat[6].loai = kLOAI_VAT_THE__BOOL;
   danhSachVat[6].mucDichBool = 1;
   danhSachVat[6].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6]), &phongTo, &quaternion, &viTri );
   danhSachVat[6].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[6].soHoaTiet = kHOA_TIET__CA_RO;

   danhSachVat[6].soLuongVatThe = 2;
   danhSachVat[6].danhSachVatThe = malloc( sizeof( VatThe )*2 );
   
   viTri.x = 0.0f;   viTri.y = 0.0f;  viTri.z = 0.0f;
   danhSachVat[6].danhSachVatThe[0].hinhDang.hop = datHop( 149.9f, 29.9f, 149.9f, &(danhSachVat[6].danhSachVatThe[0].baoBiVT) );
   danhSachVat[6].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[6].danhSachVatThe[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6].danhSachVatThe[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[6].danhSachVatThe[0].giaTri = 1;
   
   // ---- khắc hình cầu lớn
   viTri.x = 50.0f;   viTri.y = 82.0f;  viTri.z = -20.0f;
   danhSachVat[6].danhSachVatThe[1].hinhDang.hinhCau = datHinhCau( 95.0f, &(danhSachVat[6].danhSachVatThe[1].baoBiVT) );
   danhSachVat[6].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[6].danhSachVatThe[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6].danhSachVatThe[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[6].danhSachVatThe[1].giaTri = -1;
   
   // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
   tinhBaoBiVTChoVatTheGhep( &(danhSachVat[6]) );
   
   viTri.x = -300.0f;      viTri.y = 120.0f;        viTri.z = 1050.0f;
   danhSachVat[7].hinhDang.hop = datHop( 79.9f, 39.9f, 79.9f, &(danhSachVat[7].baoBiVT) );
   danhSachVat[7].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[7].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[7]), &phongTo, &quaternion, &viTri );
   danhSachVat[7].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[7].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = -250.0f;      viTri.y = 110.0f;        viTri.z = 1250.0f;
   danhSachVat[8].hinhDang.hop = datHop( 79.9f, 19.9f, 79.9f, &(danhSachVat[8].baoBiVT) );
   danhSachVat[8].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[8].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[8]), &phongTo, &quaternion, &viTri );
   danhSachVat[8].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[8].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.x = -150.0f;      viTri.y = 110.0f;        viTri.z = 1450.0f;
   danhSachVat[9].hinhDang.hop = datHop( 129.9f, 19.9f, 129.9f, &(danhSachVat[9].baoBiVT) );
   danhSachVat[9].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[9].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[9]), &phongTo, &quaternion, &viTri );
   danhSachVat[9].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[9].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.x = -300.0f;      viTri.y = 110.0f;        viTri.z = 650.0f;
   danhSachVat[10].loai = kLOAI_VAT_THE__BOOL;
   danhSachVat[10].mucDichBool = 1;
   danhSachVat[10].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[10]), &phongTo, &quaternion, &viTri );
   danhSachVat[10].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[10].soHoaTiet = kHOA_TIET__CA_RO;
   
   danhSachVat[10].soLuongVatThe = 3;
   danhSachVat[10].danhSachVatThe = malloc( sizeof( VatThe )*3 );
   
   viTri.x = 0.0f;   viTri.y = 0.0f;  viTri.z = 0.0f;
   danhSachVat[10].danhSachVatThe[0].hinhDang.hop = datHop( 259.9f, 19.9f, 259.9f, &(danhSachVat[10].danhSachVatThe[0].baoBiVT) );
   danhSachVat[10].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[10].danhSachVatThe[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[10].danhSachVatThe[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[10].danhSachVatThe[0].giaTri = 1;

   // ---- khắc hình cầu lớn
   viTri.x = -50.0f;   viTri.y = 55.0f;  viTri.z = -70.0f;
   danhSachVat[10].danhSachVatThe[1].hinhDang.hinhCau = datHinhCau( 60.0f, &(danhSachVat[10].danhSachVatThe[1].baoBiVT) );
   danhSachVat[10].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[10].danhSachVatThe[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[10].danhSachVatThe[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[10].danhSachVatThe[1].giaTri = -1;
   
   // ---- khắc hình cầu nhỏ
   viTri.x = 50.0f;   viTri.y = 105.0f;  viTri.z = 50.0f;
   danhSachVat[10].danhSachVatThe[2].hinhDang.hinhCau = datHinhCau( 110.0f, &(danhSachVat[10].danhSachVatThe[2].baoBiVT) );
   danhSachVat[10].danhSachVatThe[2].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[10].danhSachVatThe[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[10].danhSachVatThe[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[10].danhSachVatThe[2].giaTri = -1;
   // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
   tinhBaoBiVTChoVatTheGhep( &(danhSachVat[10]) );

   viTri.x = 130.0f;      viTri.y = 110.0f;        viTri.z = 900.0f;
   danhSachVat[11].loai = kLOAI_VAT_THE__BOOL;
   danhSachVat[11].mucDichBool = 1;
   danhSachVat[11].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[11]), &phongTo, &quaternion, &viTri );
   danhSachVat[11].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[11].soHoaTiet = kHOA_TIET__CA_RO;
   
   danhSachVat[11].soLuongVatThe = 2;
   danhSachVat[11].danhSachVatThe = malloc( sizeof( VatThe )*2 );
   
   viTri.x = 0.0f;   viTri.y = 0.0f;  viTri.z = 0.0f;
   danhSachVat[11].danhSachVatThe[0].hinhDang.hop = datHop( 209.9f, 19.9f, 159.9f, &(danhSachVat[11].danhSachVatThe[0].baoBiVT) );
   danhSachVat[11].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[11].danhSachVatThe[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[11].danhSachVatThe[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[11].danhSachVatThe[0].giaTri = 1;
   
   // ---- hình cầu lớn
   viTri.x = -80.0f;   viTri.y = 170.0f;  viTri.z = -60.0f;
   danhSachVat[11].danhSachVatThe[1].hinhDang.hinhCau = datHinhCau( 180.0f, &(danhSachVat[11].danhSachVatThe[1].baoBiVT) );
   danhSachVat[11].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[11].danhSachVatThe[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[11].danhSachVatThe[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[11].danhSachVatThe[1].giaTri = -1;
   // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
   tinhBaoBiVTChoVatTheGhep( &(danhSachVat[11]) );

   viTri.x = -90.0f;      viTri.y = 106.0f;        viTri.z = 1065.0f;
   danhSachVat[12].hinhDang.hop = datHop( 79.9f, 11.9f, 69.9f, &(danhSachVat[12].baoBiVT) );
   danhSachVat[12].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[12].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[12]), &phongTo, &quaternion, &viTri );
   danhSachVat[12].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[12].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.x = -100.0f;      viTri.y = 106.0f;        viTri.z = 850.0f;
   danhSachVat[13].hinhDang.hop = datHop( 79.9f, 11.9f, 69.9f, &(danhSachVat[13].baoBiVT) );
   danhSachVat[13].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[13].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[13]), &phongTo, &quaternion, &viTri );
   danhSachVat[13].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[13].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = 50.0f;      viTri.y = 105.0f;        viTri.z = 600.0f;
   danhSachVat[14].hinhDang.hop = datHop( 29.9f, 10.9f, 29.9f, &(danhSachVat[14].baoBiVT) );
   danhSachVat[14].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[14].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[14]), &phongTo, &quaternion, &viTri );
   danhSachVat[14].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[14].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = -80.0f;      viTri.y = 105.0f;        viTri.z = 590.0f;
   danhSachVat[15].hinhDang.hop = datHop( 59.9f, 10.9f, 59.9f, &(danhSachVat[15].baoBiVT) );
   danhSachVat[15].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[15].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[15]), &phongTo, &quaternion, &viTri );
   danhSachVat[15].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[15].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = -80.0f;      viTri.y = 105.0f;        viTri.z = 650.0f;
   danhSachVat[16].hinhDang.hop = datHop( 29.9f, 10.9f, 29.9f, &(danhSachVat[16].baoBiVT) );
   danhSachVat[16].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[16].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[16]), &phongTo, &quaternion, &viTri );
   danhSachVat[16].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[16].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = 180.0f;      viTri.y = 105.0f;        viTri.z = 1050.0f;
   danhSachVat[17].hinhDang.hop = datHop( 89.9f, 10.9f, 79.9f, &(danhSachVat[17].baoBiVT) );
   danhSachVat[17].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[17].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[17]), &phongTo, &quaternion, &viTri );
   danhSachVat[17].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[17].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.x = 450.0f;      viTri.y = 125.0f;        viTri.z = 1550.0f;
   danhSachVat[18].hinhDang.hop = datHop( 179.9f, 49.9f, 159.9f, &(danhSachVat[18].baoBiVT) );
   danhSachVat[18].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[18].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[18]), &phongTo, &quaternion, &viTri );
   danhSachVat[18].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[18].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.x = 650.0f;      viTri.y = 125.0f;        viTri.z = 2050.0f;
   danhSachVat[19].hinhDang.hop = datHop( 279.9f, 49.9f, 259.9f, &(danhSachVat[19].baoBiVT) );
   danhSachVat[19].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[19].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[19]), &phongTo, &quaternion, &viTri );
   danhSachVat[19].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[19].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.x = 150.0f;      viTri.y = 125.0f;        viTri.z = 1650.0f;
   danhSachVat[20].hinhDang.hop = datHop( 99.9f, 49.9f, 199.9f, &(danhSachVat[20].baoBiVT) );
   danhSachVat[20].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[20].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[20]), &phongTo, &quaternion, &viTri );
   danhSachVat[20].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[20].soHoaTiet = kHOA_TIET__CA_RO;

   return 21;
}


unsigned short datDayHinhTru( VatThe *danhSachVat ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;  quaternion.x = 0.0f;  quaternion.y = 0.0f;  quaternion.z = 0.0f;
   
   Vecto trucQuayMayQuayPhim;
   trucQuayMayQuayPhim.x = 1.0f;
   trucQuayMayQuayPhim.y = 0.0f;
   trucQuayMayQuayPhim.z = 0.0f;
   Quaternion quaternionXoay = datQuaternionTuVectoVaGocQuay( &trucQuayMayQuayPhim, 1.57f );

   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ==== màu đỏ và trắng 
   Mau mauDo;
   mauDo.d = 1.0f;   mauDo.l = 0.2f;   mauDo.x = 0.2f;   mauDo.dd = 1.0f;   mauDo.p = 0.0f;
   Mau mauTrang;
   mauTrang.d = 1.0f;   mauTrang.l = 0.95f;   mauTrang.x = 0.95f;   mauTrang.dd = 1.0f;   mauTrang.p = 0.0f;
   
   viTri.x = -435.0f;      viTri.y = 150.0f;        viTri.z = 500.0f;
   danhSachVat[0].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 465.0f;
   danhSachVat[1].hinhDang.hinhTru = datHinhTru( 35.0f, 115.0f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[1].soHoaTiet = kHOA_TIET__KHONG;

   viTri.z = 430.0f;
   danhSachVat[2].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[2].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[2].soHoaTiet = kHOA_TIET__KHONG;

   // ----
   viTri.x = -505.5f;      viTri.y = 220.5f;        viTri.z = 500.0f;
   danhSachVat[3].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[3].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[3].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[3].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 465.0f;
   danhSachVat[4].hinhDang.hinhTru = datHinhTru( 35.0f, 115.0f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[4].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[4].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[4].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 430.0f;
   danhSachVat[5].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[5].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[5].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[5].soHoaTiet = kHOA_TIET__KHONG;
   
   // ----
   viTri.x = -576.0f;      viTri.y = 291.0f;        viTri.z = 500.0f;
   danhSachVat[6].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[6].baoBiVT) );
   danhSachVat[6].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[6].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[6].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[6].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 465.0f;
   danhSachVat[7].hinhDang.hinhTru = datHinhTru( 35.0f, 115.0f, &(danhSachVat[7].baoBiVT) );
   danhSachVat[7].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[7].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[7]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[7].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[7].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 430.0f;
   danhSachVat[8].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[8].baoBiVT) );
   danhSachVat[8].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[8].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[8]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[8].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[8].soHoaTiet = kHOA_TIET__KHONG;
   
   // ----
   viTri.x = -667.0f;      viTri.y = 250.0f;        viTri.z = 500.0f;
   danhSachVat[9].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[9].baoBiVT) );
   danhSachVat[9].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[9].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[9]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[9].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[9].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 465.0f;
   danhSachVat[10].hinhDang.hinhTru = datHinhTru( 35.0f, 115.0f, &(danhSachVat[10].baoBiVT) );
   danhSachVat[10].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[10].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[10]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[10].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[10].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 430.0f;
   danhSachVat[11].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[11].baoBiVT) );
   danhSachVat[11].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[11].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[11]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[11].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[11].soHoaTiet = kHOA_TIET__KHONG;
   
   // ----
   viTri.x = -767.0f;      viTri.y = 250.0f;        viTri.z = 500.0f;
   danhSachVat[12].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[12].baoBiVT) );
   danhSachVat[12].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[12].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[12]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[12].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[12].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 465.0f;
   danhSachVat[13].hinhDang.hinhTru = datHinhTru( 35.0f, 115.0f, &(danhSachVat[13].baoBiVT) );
   danhSachVat[13].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[13].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[13]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[13].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[13].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 430.0f;
   danhSachVat[14].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[14].baoBiVT) );
   danhSachVat[14].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[14].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[14]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[14].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[14].soHoaTiet = kHOA_TIET__KHONG;
   
   // ----
   viTri.x = -867.0f;      viTri.y = 250.0f;        viTri.z = 500.0f;
   danhSachVat[15].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[15].baoBiVT) );
   danhSachVat[15].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[15].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[15]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[15].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[15].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 465.0f;
   danhSachVat[16].hinhDang.hinhTru = datHinhTru( 35.0f, 115.0f, &(danhSachVat[16].baoBiVT) );
   danhSachVat[16].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[16].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[16]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[16].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauDo );
   danhSachVat[16].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.z = 430.0f;
   danhSachVat[17].hinhDang.hinhTru = datHinhTru( 50.0f, 35.0f, &(danhSachVat[17].baoBiVT) );
   danhSachVat[17].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[17].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[17]), &phongTo, &quaternionXoay, &viTri );
   danhSachVat[17].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauTrang );
   danhSachVat[17].soHoaTiet = kHOA_TIET__KHONG;

   return 18;
}

unsigned short datNapDayHinhTru( VatThe *danhSachVat ) {
   
   Vecto trucXoay;
   trucXoay.x = 1.0f;
   trucXoay.y = 0.0f;
   trucXoay.z = 0.0f;
   Quaternion quaternionXoay = datQuaternionTuVectoVaGocQuay( &trucXoay, 1.57f );
   Quaternion quaternionKhongXoay;
   quaternionKhongXoay.w = 1.0f;  quaternionKhongXoay.x = 0.0f;  quaternionKhongXoay.y = 0.0f;  quaternionKhongXoay.z = 0.0f;
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto viTriTuongDoiHinhNon;
   viTriTuongDoiHinhNon.x = 0.0f;       viTriTuongDoiHinhNon.y = 0.0f;       viTriTuongDoiHinhNon.z = 0.0f;
   Vecto viTriTuongDoiHop;
   viTriTuongDoiHop.x = 0.0f;       viTriTuongDoiHop.y = 0.1f;       viTriTuongDoiHop.z = 0.0f;

   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ==== màu đỏ và trắng
   Mau mauDo;
   mauDo.d = 1.0f;   mauDo.l = 0.2f;   mauDo.x = 0.2f;   mauDo.dd = 1.0f;   mauDo.p = 0.0f;
   Mau mauTrang;
   mauTrang.d = 1.0f;   mauTrang.l = 0.95f;   mauTrang.x = 0.95f;   mauTrang.dd = 1.0f;   mauTrang.p = 0.0f;
   
   float mangViTri_x[] = {-435.0f, -505.5f, -576.0f, -667.0f, -767.0f, -867.0f};
   float mangViTri_y[] = {150.0f, 220.5f, 291.0f, 250.0f, 250.0f, 250.0f};
   
   unsigned short soLuongVatThe = 0;
   while ( soLuongVatThe < 12 ) {
      // ---- chọn màu
      Mau mau;
      if( (soLuongVatThe >> 1) & 0x01 )
         mau = mauDo;
      else
         mau = mauTrang;
      
      // ---- chọn góc xoay sẵn cho hộp
      float gocXoay = 6.28f*rand()/kSO_NGUYEN_TOI_DA;
      trucXoay.x = 0.0;   trucXoay.y = 0.0f;    trucXoay.z = 1.0f;
      Quaternion quaternionTuongDoi = datQuaternionTuVectoVaGocQuay( &trucXoay, gocXoay );
      Quaternion xoayGom = nhanQuaternionVoiQuaternion( &quaternionXoay, &quaternionTuongDoi );
      
      // ---- VẬT THỂ BOOL
      viTri.x = mangViTri_x[soLuongVatThe >> 1];      viTri.y = mangViTri_y[soLuongVatThe >> 1];      viTri.z = 524.5f;
      danhSachVat[soLuongVatThe].loai = kLOAI_VAT_THE__BOOL;
      danhSachVat[soLuongVatThe].mucDichBool = 1;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &xoayGom, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      
      danhSachVat[soLuongVatThe].soLuongVatThe = 2;
      danhSachVat[soLuongVatThe].danhSachVatThe = malloc( 2*sizeof(VatThe) );
      
      // ---- THÀNH PHẦN VẬT THỂ BOOL
      danhSachVat[soLuongVatThe].danhSachVatThe[0].hinhDang.hinhNon = datHinhNon( 35.0f, 30.0f, 4.0f, &(danhSachVat[soLuongVatThe].danhSachVatThe[0].baoBiVT) );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soLuongVatThe].danhSachVatThe[0].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe].danhSachVatThe[0]), &phongTo, &quaternionKhongXoay, &viTriTuongDoiHinhNon );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].danhSachVatThe[0].giaTri = 1;

      viTriTuongDoiHop.y = 0.1f;
      danhSachVat[soLuongVatThe].danhSachVatThe[1].hinhDang.hop = datHop( 71.0f, 4.0f, 12.0f, &(danhSachVat[soLuongVatThe].danhSachVatThe[1].baoBiVT) );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].danhSachVatThe[1].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe].danhSachVatThe[1]), &phongTo, &quaternionKhongXoay, &viTriTuongDoiHop );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].danhSachVatThe[1].giaTri = -1;
      // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
      tinhBaoBiVTChoVatTheGhep( &(danhSachVat[soLuongVatThe]) );
      soLuongVatThe++;
      
      // ----
      viTri.x = mangViTri_x[soLuongVatThe >> 1];      viTri.y = mangViTri_y[soLuongVatThe >> 1];      viTri.z = 405.5f;
      danhSachVat[soLuongVatThe].loai = kLOAI_VAT_THE__BOOL;
      danhSachVat[soLuongVatThe].mucDichBool = 1;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternionXoay, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      
      danhSachVat[soLuongVatThe].soLuongVatThe = 2;
      danhSachVat[soLuongVatThe].danhSachVatThe = malloc( 2*sizeof(VatThe) );
      
      viTriTuongDoiHop.y = -0.1f;
      danhSachVat[soLuongVatThe].danhSachVatThe[0].hinhDang.hinhNon = datHinhNon( 30.0f, 35.0f, 4.0f, &(danhSachVat[soLuongVatThe].danhSachVatThe[0].baoBiVT) );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soLuongVatThe].danhSachVatThe[0].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe].danhSachVatThe[0]), &phongTo, &quaternionKhongXoay, &viTriTuongDoiHinhNon );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].danhSachVatThe[0].giaTri = 1;
      
      danhSachVat[soLuongVatThe].danhSachVatThe[1].hinhDang.hop = datHop( 71.0f, 4.0f, 12.0f, &(danhSachVat[soLuongVatThe].danhSachVatThe[1].baoBiVT) );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].danhSachVatThe[1].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe].danhSachVatThe[1]), &phongTo, &quaternionKhongXoay, &viTriTuongDoiHop );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].danhSachVatThe[1].giaTri = -1;
      // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
      tinhBaoBiVTChoVatTheGhep( &(danhSachVat[soLuongVatThe]) );
      
      soLuongVatThe++;
   }
   return soLuongVatThe;
}

unsigned short datHinhCauKhongLoXoay( VatThe *danhSachVat ) {

   Vecto trucXoay;
   trucXoay.x = 0.0f;    trucXoay.y = -1.0f;   trucXoay.z = 0.0f;
   Quaternion quaternion = datQuaternionTuVectoVaGocQuay( &trucXoay, 0.7f );
   
   Vecto viTri;
   viTri.x = 0.0f;       viTri.y = 0.0f;       viTri.z = 0.0f;
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ==== màu mặt đường
   Mau mauDat;
   mauDat.d = 1.0f;   mauDat.l = 0.0f;   mauDat.x = 0.0f;   mauDat.dd = 1.0f;   mauDat.p = 0.0f;
   Mau mau1;
   mau1.d = 1.0f;   mau1.l = 0.95f;   mau1.x = 0.95f;   mau1.dd = 1.0f;    mau1.p = 0.0f;
   
   // ---- mặt đường đi
   viTri.x = -700.0f;      viTri.y = 400.0f;        viTri.z = 900.0f;
   danhSachVat[0].hinhDang.hinhCau = datHinhCau( 215.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mauDat, &mau1, 40.0f, 40.0f, 40.0f );
   danhSachVat[0].soHoaTiet = kHOA_TIET__CA_RO;

   return 1;
}

unsigned short datHemVucNui( VatThe *danhSachVat ) {

   unsigned short soLuongVatThe = 0;
   Vecto viTri;
   float dichLen;
   
   // ---- tường hẻm núi bên +z
   viTri.x = 730.0f;    viTri.z = 260.0f;
   dichLen = -177.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 125.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = 560.0f;    viTri.z = 240.0f;
   dichLen = -174.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 65.0f, &(danhSachVat[soLuongVatThe]) );
   
   viTri.x = 380.0f;    viTri.z = 150.0f;
   dichLen = -178.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 150.0f, &(danhSachVat[soLuongVatThe]) );
   
   viTri.x = 145.0f;    viTri.z = 235.0f;
   dichLen = -179.5f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 100.0f, &(danhSachVat[soLuongVatThe]) );
   
   viTri.x = 40.0f;    viTri.z = 195.0f;
   dichLen = -175.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 10.0f, &(danhSachVat[soLuongVatThe]) );
   
   viTri.x = -20.0f;    viTri.z = 240.0f;
   dichLen = -179.9f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 75.0f, &(danhSachVat[soLuongVatThe]) );
   
   viTri.x = -100.0f;    viTri.z = 200.0f;
   dichLen = -170.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 30.0f, &(danhSachVat[soLuongVatThe]) );
   
   viTri.x = -190.0f;    viTri.z = 220.0f;
   dichLen = -179.9f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 50.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -350.0f;    viTri.z = 180.0f;
   dichLen = -175.0f;
   soLuongVatThe +=  datTuongSong( viTri, dichLen, 130.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -420.0f;    viTri.z = 0.0f;
   dichLen = -178.5f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 80.0f, &(danhSachVat[soLuongVatThe]) );
   
   viTri.x = -500.0f;    viTri.z = 90.0f;
   dichLen = -179.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 120.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -700.0f;    viTri.z = 170.0f;
   dichLen = -175.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 90.0f, &(danhSachVat[soLuongVatThe]) );

   // ---- tường hẻm núi bên -z
   viTri.x = 730.0f;    viTri.z = -350.0f;
   dichLen = -179.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 90.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = 570.0f;    viTri.z = -390.0f;
   dichLen = -177.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 85.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = 400.0f;    viTri.z = -470.0f;
   dichLen = -178.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 110.0f, &(danhSachVat[soLuongVatThe]) );
   // ----
   viTri.x = 300.0f;    viTri.z = -390.0f;
   dichLen = -170.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 30.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = 200.0f;    viTri.z = -370.0f;
   dichLen = -172.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 40.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = 97.0f;    viTri.z = -420.0f;
   dichLen = -179.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 60.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = 0.0f;    viTri.z = -300.0f;
   dichLen = -180.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 100.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -80.0f;    viTri.z = -400.0f;
   dichLen = -170.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 60.0f, &(danhSachVat[soLuongVatThe]) );
   
   viTri.x = -170.0f;    viTri.z = -390.0f;
   dichLen = -175.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 50.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -250.0f;    viTri.z = -460.0f;
   dichLen = -177.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 80.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -450.0f;    viTri.z = -600.0f;
   dichLen = -179.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 160.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -650.0f;    viTri.z = -500.0f;
   dichLen = -178.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 70.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -735.0f;    viTri.z = -495.0f;
   dichLen = -170.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 40.0f, &(danhSachVat[soLuongVatThe]) );

   viTri.x = -850.0f;    viTri.z = -400.0f;
   dichLen = -179.0f;
   soLuongVatThe += datTuongSong( viTri, dichLen, 100.0f, &(danhSachVat[soLuongVatThe]) );

   return soLuongVatThe;
}

//      bánKính
//     |<---->|
//
// 280 +--(0)-+-+ 5
// 270 +------+--+ 7
//     |  (1) |  |
// 250 +--(2)-+--+
// 245 +--(3)-+-+
// 240 +--(4)-+--+
// 220 +------+--+
//     |  (5) |   \
// 180 +------+----+ 40
//     |  (6) |    |
// 140 +--(7)-+----+
// 130 +------+----\ 32
//     |  (8) |    |
// 110 +------+----+
//     |      |     \
//  90 +------+------+  90
//  80 +------+------+
//     |  (9) |       \
//  75 +------+--------+  130
//  70 +------+--------+
//  68 +------+----------+ 140
//     |      |          |
//     | (10) |          |
//   0 +------+----------+

//  Chỉ xài tọa độ (x; z) của vị trí
unsigned short datTuongSong( Vecto viTri, float dichLen, float banKinh, VatThe *danhSachVat ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;

   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   Mau mau0;
   Mau mau1;

   // ---- xây từ trên đi xuống
   mau0.d = 1.0f;   mau0.l = 0.6f;  mau0.x = 0.6f;   mau0.dd = 1.0f;   mau0.p = 0.0f;
   mau1.d = 1.0f;   mau1.l = 1.0f;  mau1.x = 1.0f;   mau1.dd = 1.0f;   mau1.p = 0.0f;
   
   viTri.y = 275.0f + dichLen;
   danhSachVat[0].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 0.0f, 10.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[0].soHoaTiet = kHOA_TIET__CA_RO;

   // ----
   viTri.y = 260.0f + dichLen;
   danhSachVat[1].hinhDang.hinhTru = datHinhTru( banKinh + 7.0f, 20.0f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 20.0f, 15.0f, 40.0f );
   danhSachVat[1].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.y = 247.5f + dichLen;
   danhSachVat[2].hinhDang.hinhNon = datHinhNon( banKinh + 2.0f, banKinh + 7.0f, 5.0f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[2].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[2].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.y = 242.5f + dichLen;
   danhSachVat[3].hinhDang.hinhNon = datHinhNon( banKinh + 7.0f, banKinh + 4.0f, 5.0f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[3].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternion, &viTri );
   danhSachVat[3].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[3].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.y = 230.0f + dichLen;
   danhSachVat[4].hinhDang.hinhTru = datHinhTru( banKinh + 7.0f, 20.0f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[4].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternion, &viTri );
   danhSachVat[4].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 20.0f, 15.0f, 40.0f );
   danhSachVat[4].soHoaTiet = kHOA_TIET__CA_RO;
   // ----
   viTri.y = 200.0f + dichLen;
   danhSachVat[5].hinhDang.hinhNon = datHinhNon( banKinh + 40.0f, banKinh + 7.0f, 40.0f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[5].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[5].soHoaTiet = kHOA_TIET__CA_RO;
 
   viTri.y = 160.0f + dichLen;
   danhSachVat[6].hinhDang.hinhTru = datHinhTru( banKinh + 40.0f, 40.0f, &(danhSachVat[6].baoBiVT) );
   danhSachVat[6].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[6].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6]), &phongTo, &quaternion, &viTri );
   danhSachVat[6].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[6].soHoaTiet = kHOA_TIET__CA_RO;
   // -- nét nhỏ
   viTri.y = 135.0f + dichLen;
   danhSachVat[7].hinhDang.hinhNon = datHinhNon( banKinh + 40.0f, banKinh + 32.0f, 10.0f, &(danhSachVat[7].baoBiVT) );
   danhSachVat[7].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[7].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[7]), &phongTo, &quaternion, &viTri );
   danhSachVat[7].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[7].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.y = 120.0f + dichLen;
   danhSachVat[8].hinhDang.hinhTru = datHinhTru( banKinh + 40.0f, 30.0f, &(danhSachVat[8].baoBiVT) );
   danhSachVat[8].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[8].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[8]), &phongTo, &quaternion, &viTri );
   danhSachVat[8].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[8].soHoaTiet = kHOA_TIET__CA_RO;
  
   viTri.y = 100.0f + dichLen;
   danhSachVat[9].hinhDang.hinhNon = datHinhNon( banKinh + 80.0f, banKinh + 40.0f, 20.0f, &(danhSachVat[9].baoBiVT) );
   danhSachVat[9].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[9].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[9]), &phongTo, &quaternion, &viTri );
   danhSachVat[9].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[9].soHoaTiet = kHOA_TIET__CA_RO;

   viTri.y = 85.0f + dichLen;
   danhSachVat[10].hinhDang.hinhTru = datHinhTru( banKinh + 80.0f, 10.0f, &(danhSachVat[10].baoBiVT) );
   danhSachVat[10].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[10].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[10]), &phongTo, &quaternion, &viTri );
   danhSachVat[10].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[10].soHoaTiet = kHOA_TIET__CA_RO;
   // ----
   viTri.y = 77.5f + dichLen;
   danhSachVat[11].hinhDang.hinhNon = datHinhNon( banKinh + 120.0f, banKinh + 80.0f, 5.0f, &(danhSachVat[11].baoBiVT) );
   danhSachVat[11].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[11].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[11]), &phongTo, &quaternion, &viTri );
   danhSachVat[11].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[11].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.y = 72.5f + dichLen;
   danhSachVat[12].hinhDang.hinhTru = datHinhTru( banKinh + 120.0f, 5.0f, &(danhSachVat[12].baoBiVT) );
   danhSachVat[12].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[12].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[12]), &phongTo, &quaternion, &viTri );
   danhSachVat[12].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[12].soHoaTiet = kHOA_TIET__CA_RO;
   
   viTri.y =  69.0f + dichLen;
   danhSachVat[13].hinhDang.hinhNon = datHinhNon( banKinh + 140.0f, banKinh + 120.0f, 2.0f, &(danhSachVat[13].baoBiVT) );
   danhSachVat[13].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[13].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[13]), &phongTo, &quaternion, &viTri );
   danhSachVat[13].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[13].soHoaTiet = kHOA_TIET__CA_RO;
   // ----
   viTri.y = 34.0f + dichLen;
   danhSachVat[14].hinhDang.hinhNon = datHinhNon( banKinh + 145.0f, banKinh + 140.0f, 65.0f, &(danhSachVat[14].baoBiVT) );
   danhSachVat[14].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[14].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[14]), &phongTo, &quaternion, &viTri );
   danhSachVat[14].hoaTiet.hoaTietCaRo = datHoaTietCaRo( &mau0, &mau1, 40.0f, 15.0f, 40.0f );
   danhSachVat[14].soHoaTiet = kHOA_TIET__CA_RO;
   
   return 15;
}

unsigned short datBayBongBong( VatThe *danhSachVat ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   unsigned short soBongBong = 0;
   while( soBongBong < kSO_LUONG__BONG_BONG_TRONG_BAY ) {
   
      Mau mauRanh;
      Mau mauTam;
      
      // ---- chọn màu
      unsigned char soMau = rand() & 0x7;
      if( soMau == 0 ) {
         mauRanh.d = 1.0f;   mauRanh.l = 0.0f;  mauRanh.x = 1.0f;
         mauTam.d = 1.0f;   mauTam.l = 0.6f;  mauTam.x = 0.0f;
      }
      else if( soMau == 1 ) {
         mauRanh.d = 0.0f;   mauRanh.l = 0.0f;  mauRanh.x = 1.0f;
         mauTam.d = 0.0f;   mauTam.l = 1.0f;  mauTam.x = 0.7f;
      }
      else if( soMau == 2 ) {
         mauRanh.d = 1.0f;   mauRanh.l = 0.8f;  mauRanh.x = 0.0f;
         mauTam.d = 0.5f;   mauTam.l = 0.0f;  mauTam.x = 1.0f;
      }
      else if( soMau == 3 ) {
         mauRanh.d = 1.0f;   mauRanh.l = 0.0f;  mauRanh.x = 0.2f;
         mauTam.d = 0.5f;   mauTam.l = 1.0f;  mauTam.x = 0.0f;
      }
      else if( soMau == 4 ) {
         mauRanh.d = 1.0f;   mauRanh.l = 1.0f;  mauRanh.x = 0.0f;
         mauTam.d = 0.35f;   mauTam.l = 0.0f;  mauTam.x = 1.0f;
      }
      else if( soMau == 5 ) {
         mauTam.d = 1.0f;   mauTam.l = 0.3f;  mauTam.x = 0.0f;
         mauRanh.d = 0.0f;   mauRanh.l = 1.0f;  mauRanh.x = 0.3f;
      }
      else if( soMau == 6 ) {
         mauTam.d = 1.0f;   mauTam.l = 0.0f;  mauTam.x = 1.0f;
         mauRanh.d = 1.0f;   mauRanh.l = 0.45f;  mauRanh.x = 0.3f;
      }
      else {
         mauRanh.d = 0.0f;   mauRanh.l = 1.0f;  mauRanh.x = 0.55f;
         mauTam.d = 1.0f;   mauTam.l = 0.0f;  mauTam.x = 1.0f;
      }
      
      mauRanh.dd = 0.85;   mauRanh.p = 0.3f;
      mauTam.dd = 0.1f;   mauTam.p = 0.3f;

      // ---- chọn vị trí ngẫu nhiên, tương đối với đường Bezier cho bong bóng
      Vecto viTri;
      float cachDonViHoa = (rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA)/4.0f - 0.5f;  // <-----
      viTri.x = 300.0f*cachDonViHoa;

      cachDonViHoa = cachDonViHoa = (rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA)/4.0f - 0.5f;  // <-----
      viTri.y = 300.0f*cachDonViHoa;
      
      cachDonViHoa = cachDonViHoa = (rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA + rand()/kSO_NGUYEN_TOI_DA)/4.0f - 0.5f;
      viTri.z = 300.0f*cachDonViHoa;
      
      // --- chọn tham số Bezier ngẫu nhiên, giúp lan bong bóng rộng rãi trên đường đi
      danhSachVat[soBongBong].thamSoBezier = 4.0f*rand()/kSO_NGUYEN_TOI_DA;
   
      // ---- đặt vật thể bong bóng
      float banKinhBongBong = 3.0f + 10.0f*rand()/kSO_NGUYEN_TOI_DA;
      danhSachVat[soBongBong].hinhDang.hinhCau = datHinhCau( banKinhBongBong, &(danhSachVat[soBongBong].baoBiVT) );
      danhSachVat[soBongBong].loai = kLOAI_HINH_DANG__HINH_CAU;
      danhSachVat[soBongBong].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soBongBong]), &phongTo, &quaternion, &viTri );
      danhSachVat[soBongBong].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauRanh, &mauTam );
      danhSachVat[soBongBong].soHoaTiet = kHOA_TIET__DI_HUONG;
      // ---- xài cho tính vị trí tho đường Bezier
      danhSachVat[soBongBong].viTriDau = viTri;
      soBongBong++;
   }

   return kSO_LUONG__BONG_BONG_TRONG_BAY;
}

unsigned short datCong( VatThe *danhSachVat, Vecto viTriDay ) {

   Vecto viTriCot;
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   unsigned short soLuongVatThe = 0;
   
   // ---- 4 cột bên -x (cho đấy)
   // ---- cột số 1
   viTriCot = viTriDay;
   viTriCot.x -= 13.0f;
   viTriCot.z += 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   // ---- cột số 2
   viTriCot.z = viTriDay.z - 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   // ---- cột số 3
   viTriCot.x = viTriDay.x - 23.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   // ---- cột số 4
   viTriCot.z = viTriDay.z + 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   
   // ---- 4 cột bên +x (cho đấy)
   // ---- cột số 0
   viTriCot.x = viTriDay.x + 13.0f;
   viTriCot.z = viTriDay.z + 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   // ---- cột số 1
   viTriCot.z = viTriDay.z - 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   // ---- cột số 2
   viTriCot.x = viTriDay.x + 23.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   // ---- cột số 3
   viTriCot.z = viTriDay.z + 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   

   // ---- mái bên -x
   Mau mau;
   mau.d = 1.0f;    mau.l = 0.8f;    mau.x = 0.8f;    mau.dd = 1.0f;   mau.p = 0.1f;
   
   Mau mauOc0;
   Mau mauOc1;
   Mau mauOc2;
   mauOc0.d = 1.0f;     mauOc0.l = 1.0f;      mauOc0.x = 1.0f;     mauOc0.dd = 1.0f;    mauOc0.p = 0.1f;
   mauOc1.d = 1.0f;     mauOc1.l = 0.5f;      mauOc1.x = 0.5f;     mauOc1.dd = 1.0f;    mauOc1.p = 0.1f;
   mauOc2.d = 1.0f;     mauOc2.l = 0.9f;      mauOc2.x = 0.9f;     mauOc2.dd = 1.0f;    mauOc2.p = 0.1f;

   Vecto viTri;
   viTri.x = viTriDay.x - 20.0f;
   viTri.y = viTriDay.y + 13.5f;
   viTri.z = viTriDay.z + 5.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 19.5f, 0.6f, 5.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;

   viTri.z = viTriDay.z - 5.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 19.5f, 0.6f, 5.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;

   viTri.y += 0.8f;
   viTri.z = viTriDay.z;
   Vecto huongNgang;
   huongNgang.x = 1.0f;   huongNgang.y = 0.0f;   huongNgang.z = 0.0f;
   Vecto huongDoc;
   huongDoc.x = 0.0f;   huongDoc.y = 1.0f;   huongDoc.z = 0.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 20.5f, 1.0f, 16.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mau, &mauOc0, &mauOc1, &mauOc2, 0.1667f, 0.51f, 0.5f, 2.2f, 2.2f, 0.2f, 0.0f, 0.8f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   soLuongVatThe++;
   
   viTri.x = viTriDay.x - 13.0f;
   viTri.y += 0.8f;
   viTri.z = viTriDay.z;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 5.5f, 0.6f, 15.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   // ---- vòng tròn lớn dưới kim tư tháp
   viTri.x = viTriDay.x - 22.0f;
   viTri.y += 0.1f;
   danhSachVat[soLuongVatThe].hinhDang.hinhTru = datHinhTru( 7.0f, 0.8f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTiaPhai = datHoaTietQuanSongTiaPhai( &mau, &mauOc0, &mauOc1, &mauOc2, 0.3333f, 0.3333f, 0.3333f, 0.5f, 0.1f, 0.0f, 8,  4.0f, 8.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_TIA_PHAI;
   soLuongVatThe++;

   mau.d = 1.0f;   mau.l = 0.3f;   mau.x = 0.3f;
   viTri.y += 0.7f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( 6.0f, 6.5f, 0.6f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTiaPhai = datHoaTietQuanSongTiaPhai( &mau, &mauOc0, &mauOc1, &mauOc2, 0.3333f, 0.3333f, 0.3333f, 0.5f, 0.1f, 0.0f, 24,  4.0f, 8.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_TIA_PHAI;
   soLuongVatThe++;
   
   viTri.y += 0.6f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( 6.5f, 6.0f, 0.6f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTiaPhai = datHoaTietQuanSongTiaPhai( &mau, &mauOc0, &mauOc1, &mauOc2, 0.3333f, 0.3333f, 0.3333f, 0.5f, 0.1f, 0.0f, 24,  4.0f, 8.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_TIA_PHAI;
   soLuongVatThe++;
   
   // ---- hai vòng tròn nhỏ
   viTri.x = viTriDay.x - 28.5f;
   viTri.y = viTriDay.y + 15.0f;
   viTri.z = viTriDay.z + 6.5f;
   danhSachVat[soLuongVatThe].hinhDang.hinhTru = datHinhTru( 1.3f, 0.4f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   viTri.z = viTriDay.z - 6.5f;
   danhSachVat[soLuongVatThe].hinhDang.hinhTru = datHinhTru( 1.3f, 0.4f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;

   // ===== mái bên +x
   mau.d = 1.0f;     mau.l = 0.8f;   mau.x = 0.8f;
   viTri.x = viTriDay.x + 20.0f;
   viTri.y = viTriDay.y + 13.5f;
   viTri.z = viTriDay.z + 5.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 19.5f, 0.6f, 5.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   viTri.z = viTriDay.z - 5.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 19.5f, 0.6f, 5.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   viTri.y += 0.8f;
   viTri.z = viTriDay.z;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 20.5f, 1.0f, 16.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mau, &mauOc0, &mauOc1, &mauOc2, 0.1667f, 0.51f, 0.5f, 2.0f, 2.0f, 0.2f, 0.0f, 0.8f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   soLuongVatThe++;
   
   viTri.x = viTriDay.x + 13.0f;
   viTri.y += 0.8f;
   viTri.z = viTriDay.z;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 5.5f, 0.6f, 15.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   // ---- vòng tròn lớn dưới kim tư tháp
   viTri.x = viTriDay.x + 22.0f;
   viTri.y += 0.1f;
   danhSachVat[soLuongVatThe].hinhDang.hinhTru = datHinhTru( 7.0f, 0.8f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTiaPhai = datHoaTietQuanSongTiaPhai( &mau, &mauOc0, &mauOc1, &mauOc2, 0.3333f, 0.3333f, 0.3333f, 0.5f, 0.1f, 0.0f, 8,  4.0f, 8.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_TIA_PHAI;
   soLuongVatThe++;
   
   mau.d = 1.0f;   mau.l = 0.3f;   mau.x = 0.3f;   mau.dd = 1.0f;
   viTri.y += 0.7f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( 6.0f, 6.5f, 0.6f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTiaPhai = datHoaTietQuanSongTiaPhai( &mau, &mauOc0, &mauOc1, &mauOc2, 0.3333f, 0.3333f, 0.3333f, 0.5f, 0.1f, 0.0f, 8,  4.0f, 8.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_TIA_PHAI;
   soLuongVatThe++;

   viTri.y += 0.6f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( 6.5f, 6.0f, 0.6f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTiaPhai = datHoaTietQuanSongTiaPhai( &mau, &mauOc0, &mauOc1, &mauOc2, 0.3333f, 0.3333f, 0.3333f, 0.5f, 0.1f, 0.0f, 8,  4.0f, 8.0f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_TIA_PHAI;
   soLuongVatThe++;

   // ---- hai vòng tròn nhỏ
   viTri.x = viTriDay.x + 28.5f;
   viTri.y = viTriDay.y + 15.0f;
   viTri.z = viTriDay.z + 6.5f;
   danhSachVat[soLuongVatThe].hinhDang.hinhTru = datHinhTru( 1.3f, 0.4f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   viTri.z = viTriDay.z - 6.5f;
   danhSachVat[soLuongVatThe].hinhDang.hinhTru = datHinhTru( 1.3f, 0.4f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;

   // ==== 2 cột bên -x (trên)
   // ---- cột số 1
   viTriCot = viTriDay;
   viTriCot.x -= 13.0f;
   viTriCot.y += 15.4f;
   viTriCot.z += 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   // ---- cột số 2
   viTriCot.z = viTriDay.z - 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   
   // ==== 2 cột bên +x (trên)
   // ---- cột số 1
   viTriCot = viTriDay;
   viTriCot.x += 13.0f;
   viTriCot.y += 15.4f;
   viTriCot.z += 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   // ---- cột số 2
   viTriCot.z = viTriDay.z - 5.0f;
   soLuongVatThe += datCotCong( &(danhSachVat[soLuongVatThe]), viTriCot );
   
   viTri = viTriDay;
   viTri.y = viTriDay.y + 29.1f;
   soLuongVatThe += datMaiDinhCong( &(danhSachVat[soLuongVatThe]), viTri );
   return soLuongVatThe;
}

unsigned short datCotCong( VatThe *danhSachVat, Vecto viTriDay ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;
   mau.d = 0.0f;     mau.l = 0.0f;     mau.x = 1.0f;    mau.dd = 1.0f;   mau.p = 0.1f;

   Mau mauOc0;
   Mau mauOc1;
   Mau mauOc2;

   // ---- đầy cột
   viTriDay.y += 0.25f;
   danhSachVat[0].hinhDang.hinhTru = datHinhTru( 2.7f, 0.5f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[0].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   viTriDay.y += 0.4f;
   danhSachVat[1].hinhDang.hinhNon = datHinhNon( 2.7f, 2.2f, 0.3f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[1].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[1].soHoaTiet = kHOA_TIET__KHONG;

   mauOc0.d = 0.0f;     mauOc0.l = 0.3f;      mauOc0.x = 1.0f;     mauOc0.dd = 1.0f;    mauOc0.p = 0.1f;
   mauOc1.d = 0.2f;     mauOc1.l = 0.65f;      mauOc1.x = 1.0f;     mauOc1.dd = 1.0f;    mauOc1.p = 0.1f;
   mauOc2.d = 0.65f;     mauOc2.l = 1.0f;      mauOc2.x = 1.0f;     mauOc2.dd = 1.0f;    mauOc2.p = 0.1f;
   viTriDay.y += 0.65f;
   Vecto huongNgang;
   huongNgang.x = 0.0f;   huongNgang.y = 0.0f;   huongNgang.z = 1.0f;
   Vecto huongDoc;
   huongDoc.x = 0.0f;   huongDoc.y = 1.0f;   huongDoc.z = 0.0f;
   danhSachVat[2].hinhDang.hinhTru = datHinhTru( 2.2f, 1.0f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[2].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[2].hoaTiet.hoaTietQuanSongTrucZ = datHoaTietQuanSongTrucZ( &mau, &mauOc0, &mauOc1, &mauOc2, 0.1667f, 0.5f, 0.5f, 5.0f, 0.2f, 1.0f, 8 );
   danhSachVat[2].soHoaTiet = kHOA_TIET__QUAN_SONG_TRUC_Z;
   
   viTriDay.y += 0.65f;
   danhSachVat[3].hinhDang.hinhNon = datHinhNon( 2.2f, 2.0f, 0.3f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[3].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[3].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[3].soHoaTiet = kHOA_TIET__KHONG;
   
   viTriDay.y += 0.25f;
   danhSachVat[4].hinhDang.hinhNon = datHinhNon( 2.0f, 2.2f, 0.2f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[4].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[4].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[4].soHoaTiet = kHOA_TIET__KHONG;
   
   viTriDay.y += 0.35f;
   danhSachVat[5].hinhDang.hinhTru = datHinhTru( 2.2f, 0.5f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[5].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[5].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[5].soHoaTiet = kHOA_TIET__KHONG;

   viTriDay.y += 0.4f;
   danhSachVat[6].hinhDang.hinhNon = datHinhNon( 2.0f, 1.7f, 0.5f, &(danhSachVat[6].baoBiVT) );
   danhSachVat[6].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[6].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[6]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[6].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[6].soHoaTiet = kHOA_TIET__KHONG;

   // ---- cột
   mau.d = 1.0f;     mau.l = 0.0f;     mau.x = 0.1f;    mau.dd = 1.0f;    mau.p = 0.1f;
   mauOc0.d = 0.65f;     mauOc0.l = 0.3f;     mauOc0.x = 0.0f;     mauOc0.dd = 1.0f;    mauOc0.p = 0.01f;
   mauOc1.d = 1.0f;     mauOc1.l = 0.8f;      mauOc1.x = 0.8f;     mauOc1.dd = 1.0f;    mauOc1.p = 0.05f;
   mauOc2.d = 0.5f;     mauOc2.l = 0.0f;      mauOc2.x = 0.05f;    mauOc2.dd = 1.0f;    mauOc2.p = 0.05f;
   viTriDay.y += 4.75f;
   danhSachVat[7].hinhDang.hinhTru = datHinhTru( 1.5f, 9.0f, &(danhSachVat[7].baoBiVT) );
   danhSachVat[7].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[7].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[7]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[7].hoaTiet.hoaTietQuanXoay = datHoaTietQuanXoay( &mau, &mauOc0, &mauOc1, &mauOc2, 0.1f, 0.22f, 0.15f, 1.0f, 0.0f, 1 );
   danhSachVat[7].soHoaTiet = kHOA_TIET__QUAN_XOAY;

   // ---- nắp cột
   mau.d = 1.0f;     mau.l = 0.45f;     mau.x = 0.45f;    mau.dd = 1.0f;    mau.p = 0.1f;
   viTriDay.y += 4.75f;
   danhSachVat[8].hinhDang.hinhNon = datHinhNon( 1.7f, 2.0f, 0.5f, &(danhSachVat[8].baoBiVT) );
   danhSachVat[8].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[8].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[8]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[8].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[8].soHoaTiet = kHOA_TIET__KHONG;
   
   viTriDay.y += 0.5f;
   danhSachVat[9].hinhDang.hinhTru = datHinhTru( 2.2f, 0.5f, &(danhSachVat[9].baoBiVT) );
   danhSachVat[9].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[9].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[9]), &phongTo, &quaternion, &viTriDay );
   danhSachVat[9].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[9].soHoaTiet = kHOA_TIET__KHONG;

   return 10;
}

unsigned short datMaiDinhCong( VatThe *danhSachVat, Vecto viTriDay ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;

   unsigned short soLuongVatThe = 0;
   Mau mau;
   mau.d = 1.0f;     mau.l = 0.8f;    mau.x = 0.8f;   mau.dd = 1.0f;    mau.p = 0.1f;
   Mau mauOc0;
   Mau mauOc1;
   Mau mauOc2;
   mauOc0.d = 1.0f;     mauOc0.l = 1.0f;      mauOc0.x = 1.0f;     mauOc0.dd = 1.0f;    mauOc0.p = 0.1f;
   mauOc1.d = 1.0f;     mauOc1.l = 0.5f;      mauOc1.x = 0.5f;     mauOc1.dd = 1.0f;    mauOc1.p = 0.1f;
   mauOc2.d = 1.0f;     mauOc2.l = 0.9f;      mauOc2.x = 0.9f;     mauOc2.dd = 1.0f;    mauOc2.p = 0.1f;

   Vecto huongNgang;
   huongNgang.x = 0.0f;   huongNgang.y = 0.0f;   huongNgang.z = 1.0f;
   Vecto huongDoc;
   huongDoc.x = 0.0f;   huongDoc.y = 1.0f;   huongDoc.z = 0.0f;
   
   // ---- miến dày -z
   viTri = viTriDay;
   viTri.z = viTriDay.z - 5.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 30.5f, 1.0f, 5.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mau, &mauOc0, &mauOc1, &mauOc2, 0.3f, 0.45f, 0.5f, 2.0f, 2.0f, 0.15f, 0.0f, 0.8f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   soLuongVatThe++;
   
   // ---- miến dày +z
   viTri.x = viTriDay.x;
   viTri.z = viTriDay.z + 5.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 30.5f, 1.0f, 5.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mau, &mauOc0, &mauOc1, &mauOc2, 0.3f, 0.45f, 0.5f, 2.0f, 2.0f, 0.15f, 0.0f, 0.8f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   soLuongVatThe++;

   // -----
   viTri.x = viTriDay.x - 16.0;
   viTri.z = viTriDay.z;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 8.5f, 1.0f, 8.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mau, &mauOc0, &mauOc1, &mauOc2, 0.3f, 0.45f, 0.5f, 2.0f, 2.0f, 0.15f, 0.0f, 0.8f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   soLuongVatThe++;

   viTri.x = viTriDay.x - 6.5;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 1.0f, 1.0f, 4.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   viTri.x = viTriDay.x;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 1.0f, 1.0f, 4.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;

   viTri.x = viTriDay.x + 6.5;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 1.0f, 1.0f, 4.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   viTri.x = viTriDay.x + 16.0;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 8.5f, 1.0f, 8.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mau, &mauOc0, &mauOc1, &mauOc2, 0.3f, 0.45f, 0.5f, 2.0f, 2.0f, 0.15f, 0.0f, 0.8f );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   soLuongVatThe++;
   
   // ---- 4 vuông góc
   mau.d = 1.0f;     mau.l = 0.5f;    mau.x = 0.5f;    mau.dd = 1.0f;
   viTri.x = viTriDay.x - 17.0f;
   viTri.z = viTriDay.z + 5.75f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 4.0f, 0.5f, 3.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;

   viTri.z = viTriDay.z - 5.75f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 4.0f, 0.5f, 3.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;
   
   viTri.x = viTriDay.x + 17.0f;
   viTri.z = viTriDay.z + 5.75f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 4.0f, 0.5f, 3.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;

   viTri.z = viTriDay.z - 5.75f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 4.0f, 0.5f, 3.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   soLuongVatThe++;

   // ---- dãy kim tư tháp
   mau.d = 0.5f;   mau.l = 0.4f;   mau.x = 0.4f;   mau.dd = 0.1f;    mau.p = 0.1f;
   viTri.x = viTriDay.x - 15.0;
   viTri.y = viTriDay.y + 2.8f;
   viTri.z = viTriDay.z + 3.0f;
   unsigned char soKimTuThap = 0;
   while ( soKimTuThap < 6 ) {
      danhSachVat[soLuongVatThe].hinhDang.kimTuThap = datKimTuThap( 6.0f, 4.5f, 6.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__KIM_TU_THAP;
      danhSachVat[soLuongVatThe].chietSuat = 2.3f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      
      viTri.x += 6.0f;
      soLuongVatThe++;
      soKimTuThap++;
   }

   viTri.x = viTriDay.x - 15.0f;
   viTri.z = viTriDay.z - 3.0f;
   soKimTuThap = 0;
   while ( soKimTuThap < 6 ) {
      danhSachVat[soLuongVatThe].hinhDang.kimTuThap = datKimTuThap( 6.0f, 4.5f, 6.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__KIM_TU_THAP;
      danhSachVat[soLuongVatThe].chietSuat = 2.3f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      
      viTri.x += 6.0f;
      soLuongVatThe++;
      soKimTuThap++;
   }
   
   // ---- dãy trên
   viTri.x = viTriDay.x - 12.0f;
   viTri.y += 4.5f;
   viTri.z = viTriDay.z;
   soKimTuThap = 0;
   while ( soKimTuThap < 5 ) {
      danhSachVat[soLuongVatThe].hinhDang.kimTuThap = datKimTuThap( 6.0f, 4.5f, 6.0f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__KIM_TU_THAP;
      danhSachVat[soLuongVatThe].chietSuat = 2.3f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      
      viTri.x += 6.0f;
      soLuongVatThe++;
      soKimTuThap++;
   }

   return soLuongVatThe;
}

unsigned short datHaiKimTuThapXoay( VatThe *danhSachVat, Vecto viTriDay ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;

   Mau mau;
   mau.d = 1.0f;   mau.l = 0.7f;   mau.x = 0.7f;   mau.dd = 0.1f;   mau.p = 0.1f;
   viTri.x = viTriDay.x - 22.0;
   viTri.y = viTriDay.y + 20.5f;
   viTri.z = viTriDay.z;
   danhSachVat[0].hinhDang.kimTuThap = datKimTuThap( 8.5f, 6.5f, 8.5f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__KIM_TU_THAP;
   danhSachVat[0].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;

   viTri.x = viTriDay.x + 22.0;
   danhSachVat[1].hinhDang.kimTuThap = datKimTuThap( 8.5f, 6.5f, 8.5f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__KIM_TU_THAP;
   danhSachVat[1].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[1].soHoaTiet = kHOA_TIET__KHONG;
   
   return 2;
}

unsigned short datHaiThauKinhXoay( VatThe *danhSachVat, Vecto viTriDay ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   // ---- xài phóng to tạo thấu kính
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 0.2f;
   
   Vecto viTri;
   
   Mau mau;
   mau.d = 1.0f;   mau.l = 1.0f;   mau.x = 1.0f;   mau.dd = 0.1f;   mau.p = 0.3f;
   viTri.x = viTriDay.x - 22.0;
   viTri.y = viTriDay.y + 26.0f;
   viTri.z = viTriDay.z;
   danhSachVat[0].hinhDang.hinhCau = datHinhCau( 2.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[0].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.x = viTriDay.x + 22.0;
   danhSachVat[1].hinhDang.hinhCau = datHinhCau( 2.0f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[1].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[1].soHoaTiet = kHOA_TIET__KHONG;
   
   return 2;
}

unsigned short datVuDinhCong( VatThe *danhSachVat, Vecto viTriDay ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   
   Mau mau;
   mau.d = 0.6f;   mau.l = 0.3f;   mau.x = 0.0f;   mau.dd = 0.1f;   mau.p = 0.1f;
   viTri.x = viTriDay.x;
   viTri.y = viTriDay.y + 42.0f;
   viTri.z = viTriDay.z;
   danhSachVat[0].hinhDang.hinhTru = datHinhTru( 10.0f, 2.0f, &(danhSachVat[0].baoBiVT) );
   danhSachVat[0].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[0].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   danhSachVat[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[0].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.y += 4.0f;
   danhSachVat[1].hinhDang.hinhTru = datHinhTru( 7.0f, 2.0f, &(danhSachVat[1].baoBiVT) );
   danhSachVat[1].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[1].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   danhSachVat[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[1].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.y += 4.0f;
   danhSachVat[2].hinhDang.hinhTru = datHinhTru( 5.0f, 2.0f, &(danhSachVat[2].baoBiVT) );
   danhSachVat[2].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[2].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   danhSachVat[2].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[2].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.y += 4.0f;
   danhSachVat[3].hinhDang.hinhTru = datHinhTru( 2.5f, 2.0f, &(danhSachVat[3].baoBiVT) );
   danhSachVat[3].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[3].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternion, &viTri );
   danhSachVat[3].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[3].soHoaTiet = kHOA_TIET__KHONG;
   
   viTri.y += 4.0f;
   danhSachVat[4].hinhDang.hinhTru = datHinhTru( 1.5f, 2.0f, &(danhSachVat[4].baoBiVT) );
   danhSachVat[4].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[4].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternion, &viTri );
   danhSachVat[4].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[4].soHoaTiet = kHOA_TIET__KHONG;

   viTri.y += 4.0f;
   danhSachVat[5].hinhDang.hinhTru = datHinhTru( 1.0f, 2.0f, &(danhSachVat[5].baoBiVT) );
   danhSachVat[5].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[5].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternion, &viTri );
   danhSachVat[5].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[5].soHoaTiet = kHOA_TIET__KHONG;

   mau.d = 0.6f;   mau.l = 0.3f;   mau.x = 0.0f;   mau.dd = 1.0f;    mau.p = 0.5f;
   viTri.y += 5.0f;
   danhSachVat[6].hinhDang.batDien = datBatDien( 3.0f, 3.0f, 4.0f, &(danhSachVat[6].baoBiVT) );
   danhSachVat[6].loai = kLOAI_HINH_DANG__BAT_DIEN;
   danhSachVat[6].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[6]), &phongTo, &quaternion, &viTri );
   danhSachVat[6].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
   danhSachVat[6].soHoaTiet = kHOA_TIET__KHONG;

   return 7;
}

unsigned short datChanLaCo( VatThe *danhSachVat, Vecto viTriDay ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   viTri = viTriDay;
   viTri.y += 1.75f;  // để hình nón trên mặt đất
   
   Vecto viTriTuongDoi;
   viTriTuongDoi.x = 0.0f;   viTriTuongDoi.y = 0.0f;   viTriTuongDoi.z = 0.0f;
   
   Mau mauNen;
   mauNen.d = 0.5f;   mauNen.l = 0.0f;  mauNen.x = 0.5f;   mauNen.dd = 1.0f;   mauNen.p = 0.2f;

   Mau mauSoc;
   mauSoc.d = 1.0f;   mauSoc.l = 0.5f;  mauSoc.x = 1.0f;   mauSoc.dd = 1.0f;   mauSoc.p = 0.0f;

   unsigned short soLuongVatThe = 0;

   viTri.x -= 5.0f;

   while ( soLuongVatThe < 5 ) {

      danhSachVat[soLuongVatThe].loai = kLOAI_VAT_THE__BOOL;
      danhSachVat[soLuongVatThe].mucDichBool = 1;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietSoc = datHoaTietSoc( &mauNen, &mauSoc, 0.8f, 0.2f, kTRUC_Y );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__SOC;
      danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình

      danhSachVat[soLuongVatThe].soLuongVatThe = 2;
      danhSachVat[soLuongVatThe].danhSachVatThe = malloc( 2*sizeof(VatThe) );
      
      // vị trí tương đối
      viTriTuongDoi.y = 0.0f;
      danhSachVat[soLuongVatThe].danhSachVatThe[0].hinhDang.hinhNon = datHinhNon( 2.5f, 1.1f, 1.5f, &(danhSachVat[soLuongVatThe].danhSachVatThe[0].baoBiVT) );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soLuongVatThe].danhSachVatThe[0].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe].danhSachVatThe[0]), &phongTo, &quaternion, &viTriTuongDoi );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauNen );
      danhSachVat[soLuongVatThe].danhSachVatThe[0].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].danhSachVatThe[0].giaTri = 1;
      
      viTriTuongDoi.y = 0.0f;
      danhSachVat[soLuongVatThe].danhSachVatThe[1].hinhDang.hinhNon = datHinhNon( 0.0f, 1.0f, 2.0f, &(danhSachVat[soLuongVatThe].danhSachVatThe[1].baoBiVT) );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HINH_NON;
      danhSachVat[soLuongVatThe].danhSachVatThe[1].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe].danhSachVatThe[1]), &phongTo, &quaternion, &viTriTuongDoi );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauNen );
      danhSachVat[soLuongVatThe].danhSachVatThe[1].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].danhSachVatThe[1].giaTri = -1;
      // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
      tinhBaoBiVTChoVatTheGhep( &(danhSachVat[soLuongVatThe]) );
      
      soLuongVatThe++;
      viTri.x += 2.0f;
   }
   
//   mauNen.d = 1.0f;   mauNen.l = 0.85f;  mauNen.x = 0.95f;   mauNen.dd = 1.0f;   mauNen.p = 0.2f;

   viTri = viTriDay;
   viTri.x -= 5.0f;
   viTri.y += 0.75f;  // để hình nón trên mặt đất

   while ( soLuongVatThe < 10 ) {
      danhSachVat[soLuongVatThe].hinhDang.hinhTru = datHinhTru( 2.5f, 0.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauNen );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
      
      soLuongVatThe++;
      viTri.x += 2.0f;
   }
   
   viTri = viTriDay;
   viTri.x -= 5.0f;
   viTri.y += 0.25f;  // để hình nón trên mặt đất
   
   while ( soLuongVatThe < 15 ) {
      danhSachVat[soLuongVatThe].hinhDang.hinhTru = datHinhTru( 4.5f, 0.5f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_TRU;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauSoc );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
      
      soLuongVatThe++;
      viTri.x += 2.0f;
   }
   
   return soLuongVatThe;
}


unsigned short datLaCo( VatThe *danhSachVat, Vecto viTriDay ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   viTri = viTriDay;
   viTri.y += 1.0f;  // giữa hộp đầu
   
   Mau mau0;
   mau0.d = 1.0f;   mau0.l = 0.6f;  mau0.x = 0.5f;   mau0.dd = 1.0f;   mau0.p = 0.2f;
   Mau mau1;
   mau1.d = 0.5f;   mau1.l = 0.6f;  mau1.x = 1.0f;   mau1.dd = 1.0f;   mau1.p = 0.2f;
   
   unsigned short soLuongVatThe = 0;
   while ( soLuongVatThe < 30) {
      viTri.x = viTriDay.x - 5.0f;
      danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 2.0f, 2.0f, 0.2f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].chietSuat = 2.3f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau0 );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
      soLuongVatThe++;

      viTri.x += 4.0f;
      danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 2.0f, 2.0f, 0.2f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].chietSuat = 2.3f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau0 );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
      soLuongVatThe++;

      viTri.x += 4.0f;
      danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 2.0f, 2.0f, 0.2f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].chietSuat = 2.3f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau0 );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
      soLuongVatThe++;

      // ----
      viTri.y += 2.0f;
      viTri.x = viTriDay.x - 3.0f;
      danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 2.0f, 2.0f, 0.2f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].chietSuat = 2.3f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau1 );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
      soLuongVatThe++;
   
      viTri.x += 4.0f;
      danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 2.0f, 2.0f, 0.2f, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
      danhSachVat[soLuongVatThe].chietSuat = 2.3f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau1 );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
      soLuongVatThe++;
   
      viTri.y += 2.0f;
   }
   
   // ---- hàng cuối
   viTri.x = viTriDay.x - 5.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 2.0f, 2.0f, 0.2f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau0 );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
   soLuongVatThe++;
   
   viTri.x += 4.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 2.0f, 2.0f, 0.2f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau0 );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
   soLuongVatThe++;
   
   viTri.x += 4.0f;
   danhSachVat[soLuongVatThe].hinhDang.hop = datHop( 2.0f, 2.0f, 0.2f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HOP;
   danhSachVat[soLuongVatThe].chietSuat = 2.3f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau0 );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
   soLuongVatThe++;

   return soLuongVatThe;
}

unsigned short datDiaSauLaCo( VatThe *danhSachVat, Vecto viTriDay ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   viTri = viTriDay;

   viTri.y += 0.15f;  // để hình nón trên mặt đất
   unsigned short soLuongVatThe = 0;
   
   Mau mauVang;
   mauVang.d = 1.0f;    mauVang.l = 0.9f;    mauVang.x = 0.0f;    mauVang.dd = 1.0f;    mauVang.p = 0.5f;
   danhSachVat[soLuongVatThe].hinhDang.hinhNon = datHinhNon( 5.5f, 5.2f, 0.3f, &(danhSachVat[soLuongVatThe].baoBiVT) );
   danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_NON;
   danhSachVat[soLuongVatThe].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauVang );
   danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
   danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
   soLuongVatThe++;
   
   float mangViTriX[] = {-2.0f, -1.0f, 1.0f, 2.0f, 1.0f, -1.0f};
   float mangViTriZ[] = { 0.0f, 1.73205f, 1.73205f, 0.0f, -1.73205f, -1.73205f};
   Vecto viTriTuongDoi;
   viTriTuongDoi.x = 0.0f;    viTriTuongDoi.y = 0.0f;     viTriTuongDoi.z = 0.0f;
   viTri.y += 0.25f;
   
   // ---- các vòng tròng trên mặt dĩa
    while ( soLuongVatThe < 7 ) {
       viTri.x = viTriDay.x + mangViTriX[soLuongVatThe-1];
       viTri.z = viTriDay.z + mangViTriZ[soLuongVatThe-1];
       danhSachVat[soLuongVatThe].loai = kLOAI_VAT_THE__BOOL;
       danhSachVat[soLuongVatThe].mucDichBool = 1;
       danhSachVat[soLuongVatThe].chietSuat = 1.0f;
       datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
       danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauVang );
       danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
       danhSachVat[soLuongVatThe].viTriDau = viTri;  // cần này cho hoạt hình
       
       danhSachVat[soLuongVatThe].soLuongVatThe = 2;
       danhSachVat[soLuongVatThe].danhSachVatThe = malloc( 2*sizeof(VatThe) );
    
    // vị trí tương đối
       viTriTuongDoi.y = 0.0f;
       danhSachVat[soLuongVatThe].danhSachVatThe[0].hinhDang.hinhNon = datHinhNon( 2.8f, 2.6f, 0.2f, &(danhSachVat[soLuongVatThe].danhSachVatThe[0].baoBiVT) );
       danhSachVat[soLuongVatThe].danhSachVatThe[0].loai = kLOAI_HINH_DANG__HINH_NON;
       danhSachVat[soLuongVatThe].danhSachVatThe[0].chietSuat = 1.0f;
       datBienHoaChoVat( &(danhSachVat[soLuongVatThe].danhSachVatThe[0]), &phongTo, &quaternion, &viTriTuongDoi );
       danhSachVat[soLuongVatThe].danhSachVatThe[0].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauVang );
       danhSachVat[soLuongVatThe].danhSachVatThe[0].soHoaTiet = kHOA_TIET__KHONG;
       danhSachVat[soLuongVatThe].danhSachVatThe[0].giaTri = 1;
       
       viTriTuongDoi.y += 0.01f;  // cao hơn một chút tránh vấn đề tròn số
       danhSachVat[soLuongVatThe].danhSachVatThe[1].hinhDang.hinhNon = datHinhNon( 2.45f, 2.25f, 0.2f, &(danhSachVat[soLuongVatThe].danhSachVatThe[1].baoBiVT) );
       danhSachVat[soLuongVatThe].danhSachVatThe[1].loai = kLOAI_HINH_DANG__HINH_NON;
       danhSachVat[soLuongVatThe].danhSachVatThe[1].chietSuat = 1.0f;
       datBienHoaChoVat( &(danhSachVat[soLuongVatThe].danhSachVatThe[1]), &phongTo, &quaternion, &viTriTuongDoi );
       danhSachVat[soLuongVatThe].danhSachVatThe[1].hoaTiet.hoaTietKhong = datHoaTietKhong( &mauVang );
       danhSachVat[soLuongVatThe].danhSachVatThe[1].soHoaTiet = kHOA_TIET__KHONG;
       danhSachVat[soLuongVatThe].danhSachVatThe[1].giaTri = -1;
       // ---- đừng quên tính bao bì vật thể cho vật thể ghép/bool
       tinhBaoBiVTChoVatTheGhep( &(danhSachVat[soLuongVatThe]) );
       
       soLuongVatThe++;
    }

   
   return soLuongVatThe;
}


unsigned short datLocXoay( VatThe *danhSachVat, Vecto viTriDay ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   viTri = viTriDay;
   viTri.y += 1.0f;  // giữa hộp đầu
   
   Mau mau;
   
   float goc = 0.0f;
   float banKinhXoay = 8.0f;
   float banKinhHinhCau = 1.0f;
   float doDuc = 0.01f;

   unsigned short soLuongVatThe = 0;
   while( soLuongVatThe < kSO_LUONG__HAT_BAY_TRONG_LOC_XOAY ) {
      // ---- cỡ và màu của hình cầu lốc xoay
      if( (soLuongVatThe % 10) == 0 ) {
         mau.d = 0.3f;   mau.l = 0.3f;  mau.x = 0.3f;   mau.dd = doDuc;  mau.p = 0.1f;
         banKinhHinhCau = 1.0f;
      }
      else {
         mau.d = 0.6f;   mau.l = 0.6f;  mau.x = 0.6f;   mau.dd = doDuc;   mau.p = 0.1f;
         banKinhHinhCau = 0.7f;
      }
      float banKinhCos = banKinhXoay*cosf( goc );
      float banKinhSin = banKinhXoay*sinf( goc );
      viTri.x = viTriDay.x + banKinhCos;
      viTri.z = viTriDay.z + banKinhSin;
      danhSachVat[soLuongVatThe].hinhDang.hinhCau = datHinhCau( banKinhHinhCau, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_CAU;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau.x = viTriDay.x;  // cần này cho hoạt hình
      danhSachVat[soLuongVatThe].viTriDau.y = viTri.y;  // cần này cho hoạt hình
      danhSachVat[soLuongVatThe].viTriDau.z = viTriDay.z;  // cần này cho hoạt hình
      soLuongVatThe++;

      viTri.x = viTriDay.x - banKinhCos;
      viTri.z = viTriDay.z - banKinhSin;
      danhSachVat[soLuongVatThe].hinhDang.hinhCau = datHinhCau( banKinhHinhCau, &(danhSachVat[soLuongVatThe].baoBiVT) );
      danhSachVat[soLuongVatThe].loai = kLOAI_HINH_DANG__HINH_CAU;
      danhSachVat[soLuongVatThe].chietSuat = 1.0f;
      datBienHoaChoVat( &(danhSachVat[soLuongVatThe]), &phongTo, &quaternion, &viTri );
      danhSachVat[soLuongVatThe].hoaTiet.hoaTietKhong = datHoaTietKhong( &mau );
      danhSachVat[soLuongVatThe].soHoaTiet = kHOA_TIET__KHONG;
      danhSachVat[soLuongVatThe].viTriDau.x = viTriDay.x;  // cần này cho hoạt hình
      danhSachVat[soLuongVatThe].viTriDau.y = viTri.y;  // cần này cho hoạt hình
      danhSachVat[soLuongVatThe].viTriDau.z = viTriDay.z;  // cần này cho hoạt hình
   
      goc += 0.25f;
      viTri.y += 0.5f;
      
      if( viTri.y - viTriDay.y < 5.0f ) {  // cho đoàn gần đất trong
         doDuc = (viTri.y - viTriDay.y)*0.2f;
      }
      else      
         doDuc -= 0.0065f;  // 1.0/kSO_LUONG__HAT_BAY_TRONG_LOC_XOAY
      soLuongVatThe++;
   }

   return soLuongVatThe;
}

#pragma mark ---- NÂNG CẤP PHIM TRƯỜNG 1
void nangCapPhimTruong1( PhimTruong *phimTruong ) {
   
   //   printf( "phimTruong->soHoatHinhDau; %d\n", phimTruong->soHoatHinhDau );
   nangCapPhimTruong1_mayQuayPhim( phimTruong );
   nangCapPhimTruong1_nhanVat( phimTruong, phimTruong->soHoatHinhDau );
   
   // ---- tăng số hoạt hình
   phimTruong->soHoatHinhDau++;
}

void nangCapPhimTruong1_mayQuayPhim( PhimTruong *phimTruong ) {
   
   unsigned short soHoatHinh = phimTruong->soHoatHinhDau;
   MayQuayPhim *mayQuayPhim = &(phimTruong->mayQuayPhim);

   if( soHoatHinh < 150 ) {
      mayQuayPhim->viTri.x = 450.0f;
      mayQuayPhim->viTri.y = 250.0f;
      mayQuayPhim->viTri.z = 850.0f;

      Vecto trucQuayMayQuayPhim;
      trucQuayMayQuayPhim.x = 0.0f;
      trucQuayMayQuayPhim.y = 1.0f;
      trucQuayMayQuayPhim.z = 0.0f;
      phimTruong->mayQuayPhim.quaternion = datQuaternionTuVectoVaGocQuay( &trucQuayMayQuayPhim, 5.0f );
      quaternionQuaMaTran( &(phimTruong->mayQuayPhim.quaternion), phimTruong->mayQuayPhim.xoay );
   }
   else if( soHoatHinh < 260 ) {
      mayQuayPhim->viTri.x = 70.0f;
      mayQuayPhim->viTri.y = 120.0f;
      mayQuayPhim->viTri.z = 1075.0f;
      
      Vecto huongNhin;
      huongNhin.x = -1.0f;
      huongNhin.y = -0.0f;
      huongNhin.z = -0.8f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }  // ---- sát với đất
   else if( soHoatHinh < 395 ) {
      mayQuayPhim->viTri.x = 4.0f;
      mayQuayPhim->viTri.y = 101.0f;
      mayQuayPhim->viTri.z = 650.0f;

      Vecto huongNhin;
      huongNhin.x = 0.0f;
      huongNhin.y = 0.0f;
      huongNhin.z = 1.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }  // theo trái banh vai chanh và sao gai, nhìn xuống từ trời
   else if( soHoatHinh < 580 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 25.0f;   congBezier.diemQuanTri[0].y = 115.0f;   congBezier.diemQuanTri[0].z = 626.0f;
      congBezier.diemQuanTri[1].x = 30.0f;   congBezier.diemQuanTri[1].y = 120.0f;   congBezier.diemQuanTri[1].z = 546.0f;
      congBezier.diemQuanTri[2].x = 90.0f;   congBezier.diemQuanTri[2].y = 200.0f;   congBezier.diemQuanTri[2].z = 330.0f;
      congBezier.diemQuanTri[3].x = 90.0f;   congBezier.diemQuanTri[3].y = 200.0f;   congBezier.diemQuanTri[3].z = 250.0f;
      
      float thamSoBezier = (soHoatHinh - 395)/185.0f;
      Vecto viTriMayQuayPhim = tinhViTriBezier3C( &congBezier, thamSoBezier );
      
      mayQuayPhim->viTri = viTriMayQuayPhim;
      
      Vecto huongNhin;
      huongNhin.x = -1.0f;
      huongNhin.y = -0.6f - 0.25f*(soHoatHinh - 395)/185.0f;
      huongNhin.z = -0.3f;  // 1,5 là vận tốc trái banh vai chánh và sao gai
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   } // theo trái banh vai chanh và sao gai trong cổng
   else if( soHoatHinh < 720 ) {
      mayQuayPhim->viTri.x = 4.0f;
      mayQuayPhim->viTri.y = 109.0f;
      mayQuayPhim->viTri.z = 290.0f - 1.5f*(soHoatHinh - 580);
      
      Vecto huongNhin;
      huongNhin.x = 0.0f;
      huongNhin.y = 0.0f;
      huongNhin.z = -1.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 830 ) {
      mayQuayPhim->viTri.x = 20.0f;
      mayQuayPhim->viTri.y = 123.0f;
      mayQuayPhim->viTri.z = -250.0f;
      
      Vecto huongNhin;
      huongNhin.x = -0.7f;
      huongNhin.y = -0.5f;
      huongNhin.z = 1.5f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 900 ) {
      mayQuayPhim->viTri.x = 5.0f;
      mayQuayPhim->viTri.y = 108.0f;
      mayQuayPhim->viTri.z = -220.0f;
      
      Vecto huongNhin;
      huongNhin.x = -0.1f;
      huongNhin.y = -0.1f;
      huongNhin.z = -1.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   // ---- nhìn -z khi vào bong bóng (khi trở lại kiếm trái banh vai tranh)
   else if( soHoatHinh < 1000 ) {
      mayQuayPhim->viTri.x = 30.0f;
      mayQuayPhim->viTri.y = 133.0f;
      mayQuayPhim->viTri.z = -250.0f;
      
      Vecto huongNhin;
      huongNhin.x = -1.0f;
      huongNhin.y = -1.0f;
      huongNhin.z = 1.5f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   // ---- nhìn tới -z khi chùm ra dòng bong bóng (khi trở lại kiếm trái banh vai tranh)
   else if( soHoatHinh < 1150 ) {
      mayQuayPhim->viTri.x = 7.5f;
      mayQuayPhim->viTri.y = 106.0f;
      mayQuayPhim->viTri.z = 205.0f;
      
      Vecto huongNhin;
      huongNhin.x = -0.2f;
      huongNhin.y = -0.3f;
      huongNhin.z = -1.5f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if(soHoatHinh < 1170 ) {   // ---- nhìn tới +z
      mayQuayPhim->viTri.x = 3.0f;
      mayQuayPhim->viTri.y = 106.0f;
      mayQuayPhim->viTri.z = 175.0f;

      Vecto huongNhin;
      huongNhin.x = sinf( 0.2f );
      huongNhin.y = -0.15f;
      huongNhin.z = cosf( 0.2f);
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   } 
   else if( soHoatHinh < 1290 ) {   // ---- nhìn tới +z, quét xem
      mayQuayPhim->viTri.x = 3.0f + 0.04f*(soHoatHinh - 1170);
      mayQuayPhim->viTri.y = 106.0f;
      mayQuayPhim->viTri.z = 175.0f;
      
      float goc = (soHoatHinh - 1170)*0.007f;
      Vecto huongNhin;
      huongNhin.x = -sinf( goc - 0.2f );
      huongNhin.y = -0.15f;
      huongNhin.z = cosf( goc - 0.2f );
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1310 ) {
      mayQuayPhim->viTri.x = 7.8f;// 3.0f + 0.04f*(1290-1170) = 3.0f + 4.8f;
      mayQuayPhim->viTri.y = 106.0f;
      mayQuayPhim->viTri.z = 175.0f;
      
      float goc = 120*0.007f;  // 130 = 1300 - 1170
      Vecto huongNhin;
      huongNhin.x = -sinf( goc - 0.2f );
      huongNhin.y = -0.15f;
      huongNhin.z = cosf( goc - 0.2f );
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1335 ) {
      mayQuayPhim->viTri.x = -3.0f;
      mayQuayPhim->viTri.y = 106.0f;
      mayQuayPhim->viTri.z = 205.0f;
      
      Vecto huongNhin;
      huongNhin.x = 0.3f;
      huongNhin.y = -0.3f;
      huongNhin.z = -1.5f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1520 ) {
      mayQuayPhim->viTri.x = 55.0f;//800.0f - 2.0f*soHoatHinh;
      mayQuayPhim->viTri.y = 245.0f;
      mayQuayPhim->viTri.z = 165.0f - (soHoatHinh - 1335)*2.0f;
      
      Vecto huongNhin;
      huongNhin.x = -0.38f;
      huongNhin.y = -1.0f;
      huongNhin.z = -0.0f;  // 1,5 là vận tốc trái banh vai chánh và sao gai
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
//      printf( "**** mayQayPhim viTriZ %5.3f\n",  mayQuayPhim->viTri.z );
   }
   else if( soHoatHinh < 1565 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 55.0f;    congBezier.diemQuanTri[0].y = 245.0f;    congBezier.diemQuanTri[0].z = -205.0f;
      congBezier.diemQuanTri[1].x = 55.0f;    congBezier.diemQuanTri[1].y = 245.0f;    congBezier.diemQuanTri[1].z = -235.0f;
      congBezier.diemQuanTri[2].x = 55.0f;    congBezier.diemQuanTri[2].y = 245.0f;    congBezier.diemQuanTri[2].z = -245.0f;
      congBezier.diemQuanTri[3].x = 55.0f;    congBezier.diemQuanTri[3].y = 245.0f;    congBezier.diemQuanTri[3].z = -245.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 1520)/45.0f;
      mayQuayPhim->viTri = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
//      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
//      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
//      printf( "%d  may Quay Phim tocDo %5.3f\n", soHoatHinh, tocDo );

      Vecto huongNhin;
      huongNhin.x = -0.38f;
      huongNhin.y = -1.0f;
      huongNhin.z = -0.0f;  // 1,5 là vận tốc trái banh vai chánh và sao gai
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1610 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 55.0f;    congBezier.diemQuanTri[0].y = 245.0f;    congBezier.diemQuanTri[0].z = -245.0f;
      congBezier.diemQuanTri[1].x = 55.0f;    congBezier.diemQuanTri[1].y = 245.0f;    congBezier.diemQuanTri[1].z = -245.0f;
      congBezier.diemQuanTri[2].x = 55.0f;    congBezier.diemQuanTri[2].y = 245.0f;    congBezier.diemQuanTri[2].z = -235.0f;
      congBezier.diemQuanTri[3].x = 55.0f;    congBezier.diemQuanTri[3].y = 245.0f;    congBezier.diemQuanTri[3].z = -205.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 1565)/45.0f;
      mayQuayPhim->viTri = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
//      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
//      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
//      printf( "%d  may Quay Phim tocDo %5.3f\n", soHoatHinh, tocDo );
      
      Vecto huongNhin;
      huongNhin.x = -0.38f;
      huongNhin.y = -1.0f;
      huongNhin.z = -0.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
//      printf( "**** mayQayPhim viTriZ %5.3f\n",  mayQuayPhim->viTri.z );
   }
   else if( soHoatHinh < 1725 ) { // đi theo sao gai trở lại đi qua cầu nữa
      mayQuayPhim->viTri.x = 55.0f;//800.0f - 2.0f*soHoatHinh;
      mayQuayPhim->viTri.y = 245.0f;
      mayQuayPhim->viTri.z = -205.0f + 1.5f*(soHoatHinh - 1610);
      
      Vecto huongNhin;
      huongNhin.x = -0.38f;
      huongNhin.y = -1.0f;
      huongNhin.z = -0.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
//      printf( "**** mayQayPhim viTriZ %5.3f\n",  mayQuayPhim->viTri.z );
   }
   else if( soHoatHinh < 1755 ) {
      mayQuayPhim->viTri.x = 55.0f;//800.0f - 2.0f*soHoatHinh;
      mayQuayPhim->viTri.y = 245.0f;
      mayQuayPhim->viTri.z = -35.0f - (soHoatHinh - 1725);
      
      Vecto huongNhin;
      huongNhin.x = -0.38f;
      huongNhin.y = -1.0f;
      huongNhin.z = -0.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
//      printf( "**** mayQayPhim viTriZ %5.3f\n",  mayQuayPhim->viTri.z );
   }
   else if( soHoatHinh < 1770 ) {
      float phongVao = 6.0f; // phóng vào
      mayQuayPhim->viTri.x = 55.0f - phongVao*0.52f*(soHoatHinh - 1755);  // (55 - 3) = 52
      mayQuayPhim->viTri.y = 245.0f - phongVao*1.44f*(soHoatHinh - 1755); // (245 - 101) = 144
      mayQuayPhim->viTri.z = -65.0f - 6.4f*(soHoatHinh - 1755);
      
      Vecto huongNhin;
      huongNhin.x = -0.38f;
      huongNhin.y = -1.0f;
      huongNhin.z = -0.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
//      printf( "**** mayQayPhim viTriZ %5.3f\n",  mayQuayPhim->viTri.z );
   }
   else if( soHoatHinh < 1790 ) {
      float phongVao = 6.0f; // phóng vào
      mayQuayPhim->viTri.x = 11.0f;  // (55 - 3) = 52
      mayQuayPhim->viTri.y = 124.0f; // (245 - 101) = 144
      mayQuayPhim->viTri.z = -160.0f - 2.0f*(soHoatHinh - 1770);
      
      Vecto huongNhin;
      huongNhin.x = -0.38f;
      huongNhin.y = -1.0f;
      huongNhin.z = -0.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
//      printf( "**** mayQayPhim viTriZ %5.3f\n",  mayQuayPhim->viTri.z );
   }
   else if( soHoatHinh < 1890 ) {
      mayQuayPhim->viTri.x = 3.0f;//800.0f - 2.0f*soHoatHinh;
      mayQuayPhim->viTri.y = 105.0f;
      mayQuayPhim->viTri.z = -205.0f;
      
      Vecto huongNhin;
      huongNhin.x = -0.0f;
      huongNhin.y = -0.2f + 0.4f*atanf( (soHoatHinh - 1790)*0.05f )/3.1415926f;
      huongNhin.z = -1.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else if( soHoatHinh < 1920 ) {
      mayQuayPhim->viTri.x = 3.0f;//800.0f - 2.0f*soHoatHinh;
      mayQuayPhim->viTri.y = 105.0f;
      mayQuayPhim->viTri.z = -205.0f - 7.0f*(soHoatHinh - 1890);
      
      Vecto huongNhin;
      huongNhin.x = -0.0f;
      huongNhin.y = -0.2f + 0.4f*atanf( (soHoatHinh - 1790)*0.05f )/3.1415926f;
      huongNhin.z = -1.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
   else {
      mayQuayPhim->viTri.x = 3.0f;//800.0f - 2.0f*soHoatHinh;
      mayQuayPhim->viTri.y = 105.0f;
      mayQuayPhim->viTri.z = -205.0f - 210;

      Vecto huongNhin;
      huongNhin.x = -0.0f;
      huongNhin.y = -0.2f + 0.4f*atanf( (soHoatHinh - 1790)*0.05f )/3.1415926f;
      huongNhin.z = -1.0f;
      dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin );
   }
/*
   // ---- cho xem từ trời xuống
   mayQuayPhim->viTri.x = 50.0f;//800.0f - 2.0f*soHoatHinh;
   mayQuayPhim->viTri.y = 280.0f;
   mayQuayPhim->viTri.z = -450.0f;
   
   Vecto huongNhin;
   huongNhin.x = -0.01f;
   huongNhin.y = -1.0f;
   huongNhin.z = -0.0f;  // 1,5 là vận tốc trái banh vai chánh và sao gai
   dinhHuongMaTranBangVectoNhin( mayQuayPhim->xoay, &huongNhin ); */
}

#pragma mark ---- Nâng Cấp Nhân Vật
void nangCapPhimTruong1_nhanVat( PhimTruong *phimTruong, unsigned short soHoatHinh ) {
   
   nangCapVaiChanh_PT_1( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__VAI_CHANH]]), soHoatHinh );
   nangCapSaoGai_PT_1( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__SAO_GAI]]), soHoatHinh );
   nangCapHinhCauKhongLoXoay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__HINH_CAU_KHONG_LO_XOAY]]) );
   nangCapNapDayHinhTru( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__NAP_DAY_HINH_TRU_DAU]]), phimTruong->nhanVat[kNHAN_VAT__NAP_DAY_HINH_TRU_CUOI] - phimTruong->nhanVat[kNHAN_VAT__NAP_DAY_HINH_TRU_DAU] );
   nangCapHaiKimTuThapXoay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__HAI_KIM_TU_THAP_XOAY]]) );
   nangCapHaiThauKinhXoay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__HAI_THAU_KINH_XOAY]]) );
   nangCapVuDinhCong( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__VU_DINH_CONG]]), soHoatHinh );
   nangCapLocXoay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__LOC_XOAY_0]]), soHoatHinh );
   nangCapLocXoay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__LOC_XOAY_1]]), soHoatHinh );
   nangCapLocXoay( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__LOC_XOAY_2]]), soHoatHinh );

   
   nangCapLaCo( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__MIEN_LA_CO_DAU]]), phimTruong->nhanVat[kNHAN_VAT__MIEN_LA_CO_CUOI] - phimTruong->nhanVat[kNHAN_VAT__MIEN_LA_CO_DAU], soHoatHinh );
   
   // ---- các bong bóng
   nangCapBayBongBong( &(phimTruong->danhSachVatThe[phimTruong->nhanVat[kNHAN_VAT__BONG_BONG_DAU_PT1]]),
                      phimTruong->nhanVat[kNHAN_VAT__BONG_BONG_CUOI_PT1] - phimTruong->nhanVat[kNHAN_VAT__BONG_BONG_DAU_PT1], soHoatHinh );
}

void nangCapVaiChanh_PT_1( VatThe *vaiChanh, unsigned short soHoatHinh ) {
   
   Vecto trucXoay;
   trucXoay.x = -1.0f;    trucXoay.y = 0.0f;   trucXoay.z = 0.0;
   Quaternion quaternion = tinhXoayChoVatThe( vaiChanh, trucXoay, 2.0f, vaiChanh->hinhDang.hinhCau.banKinh );
   //   printf( "hinhCauKhongLoXoay->hinhDang.hinhCau.banKinh %5.3f\n", hinhCauKhongLoXoay->hinhDang.hinhCau.banKinh );
   //   printf( "quaternion %5.3f %5.3f %5.3f %5.3f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z );
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ---- vị trí bắt đầu
   Vecto viTriVaiChanh;
   viTriVaiChanh.x = 3.0f; viTriVaiChanh.y = 100.2 + vaiChanh->hinhDang.hinhCau.banKinh;  viTriVaiChanh.z = 600.0f;
   if( soHoatHinh < 720 ) {
      viTriVaiChanh.z = 1400.0f - 2.0f*soHoatHinh;
   }
   else if( soHoatHinh < 740 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;    congBezier.diemQuanTri[0].y = viTriVaiChanh.y;    congBezier.diemQuanTri[0].z = -40.0f;
      congBezier.diemQuanTri[1].x = 3.0f;    congBezier.diemQuanTri[1].y = viTriVaiChanh.y;    congBezier.diemQuanTri[1].z = -53.0f;
      congBezier.diemQuanTri[2].x = 6.0f;    congBezier.diemQuanTri[2].y = viTriVaiChanh.y;    congBezier.diemQuanTri[2].z = -53.0f;
      congBezier.diemQuanTri[3].x = 6.0f;    congBezier.diemQuanTri[3].y = viTriVaiChanh.y;    congBezier.diemQuanTri[3].z = -53.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 720)/20.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/20.0f;
 //     printf( "%d  vaiChanh tocDo %5.3f\n", soHoatHinh, tocDo );
      // ---- tính quaternion mới
      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f; phapThuyenMatDat.y = 1.0f; phapThuyenMatDat.z = 0.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      quaternion = tinhXoayChoVatThe( vaiChanh, trucXoay, tocDo, vaiChanh->hinhDang.hinhCau.banKinh );
   }
   else if( soHoatHinh < 1690 ) {
      viTriVaiChanh.x = 6.0f;
      viTriVaiChanh.z = -53.0f;
   }
   // ---- chùm ra bong bóng và chạy
   else if( soHoatHinh < 1770 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 6.0f;    congBezier.diemQuanTri[0].y = viTriVaiChanh.y;    congBezier.diemQuanTri[0].z = -53.0f;
      congBezier.diemQuanTri[1].x = 6.0f;    congBezier.diemQuanTri[1].y = viTriVaiChanh.y;    congBezier.diemQuanTri[1].z = -53.0f;
      congBezier.diemQuanTri[2].x = 3.0f;    congBezier.diemQuanTri[2].y = viTriVaiChanh.y;    congBezier.diemQuanTri[2].z = -106.7f;
      congBezier.diemQuanTri[3].x = 3.0f;    congBezier.diemQuanTri[3].y = viTriVaiChanh.y;    congBezier.diemQuanTri[3].z = -160.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 1690)/80.0f;
      viTriVaiChanh = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/80.0f;
//      printf( "%d  vaiChanh viTriZ %5.3f\n", soHoatHinh, viTriVaiChanh.z );
      // ---- tính quaternion mới
      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f; phapThuyenMatDat.y = 1.0f; phapThuyenMatDat.z = 0.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      quaternion = tinhXoayChoVatThe( vaiChanh, trucXoay, tocDo, vaiChanh->hinhDang.hinhCau.banKinh );
   }

   else if( soHoatHinh < 1920 ) { // ---- chạy trống
      viTriVaiChanh.z = -160.0f - 2.0f*(soHoatHinh - 1770);
   }
   else { // ---- bị hút lên trong lốc xoay
      viTriVaiChanh.y = 100.2f + vaiChanh->hinhDang.hinhCau.banKinh + 20*atan( (soHoatHinh - 1920)*0.05f );
      
      float banKinh = expf( (viTriVaiChanh.y - 100.7f)*0.04 ) - 1.0f;
      float goc = soHoatHinh*0.3333f;
      viTriVaiChanh.x = 3.0f + 2.0f*(soHoatHinh - 1920) + banKinh*cosf( goc );
      viTriVaiChanh.z = -460.0f + 0.1f*(soHoatHinh - 1920) + banKinh*sinf( goc );
   }

//  printf( "ViTriVaiChanh %5.3f %5.3f %5.3f\n", viTriVaiChanh.x, viTriVaiChanh.y, viTriVaiChanh.z );

   datBienHoaChoVat( vaiChanh, &phongTo, &quaternion, &viTriVaiChanh );
}


void nangCapSaoGai_PT_1( VatThe *saoGai, unsigned short soHoatHinh ) {
   
   Vecto trucXoay;
   trucXoay.x = -1.0f;    trucXoay.y = 0.0f;   trucXoay.z = 0.0;
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;   quaternion.y = 0.0f;   quaternion.z = 0.0f;

   Vecto phongTo;
   phongTo.x = 1.4f;     phongTo.y = 1.4f;     phongTo.z = 1.4f;

   Vecto viTriSaoGai;
   viTriSaoGai.x = 3.0f; viTriSaoGai.y = 100.2f + kSAO_GAI__BAN_KINH*1.4f;  viTriSaoGai.z = 600.0f;
   if( soHoatHinh < 810 ) {
      viTriSaoGai.z = 1420.0f - 2.0f*soHoatHinh;
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, 2.0f, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- chảy chậm lại, vì không thấy trái banh vai chánh
   else if( soHoatHinh < 855 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;    congBezier.diemQuanTri[0].y = viTriSaoGai.y;    congBezier.diemQuanTri[0].z = -200.0f;
      congBezier.diemQuanTri[1].x = 3.0f;    congBezier.diemQuanTri[1].y = viTriSaoGai.y;    congBezier.diemQuanTri[1].z = -230.0f;
      congBezier.diemQuanTri[2].x = 3.0f;    congBezier.diemQuanTri[2].y = viTriSaoGai.y;    congBezier.diemQuanTri[2].z = -240.0f;
      congBezier.diemQuanTri[3].x = 3.0f;    congBezier.diemQuanTri[3].y = viTriSaoGai.y;    congBezier.diemQuanTri[3].z = -240.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 810)/45.0f;
      viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
//      printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );

      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f; phapThuyenMatDat.y = 1.0f; phapThuyenMatDat.z = 0.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );

      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, tocDo, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- dừng lại
   else if( soHoatHinh < 880 ) {
      viTriSaoGai.z = -240.0f;
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, 0.0f, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- tăng tốc độ; trở lại kiếm
   else if( soHoatHinh < 925 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;    congBezier.diemQuanTri[0].y = viTriSaoGai.y;    congBezier.diemQuanTri[0].z = -240.0f;
      congBezier.diemQuanTri[1].x = 3.0f;    congBezier.diemQuanTri[1].y = viTriSaoGai.y;    congBezier.diemQuanTri[1].z = -240.0f;
      congBezier.diemQuanTri[2].x = 3.0f;    congBezier.diemQuanTri[2].y = viTriSaoGai.y;    congBezier.diemQuanTri[2].z = -230.0f;
      congBezier.diemQuanTri[3].x = 3.0f;    congBezier.diemQuanTri[3].y = viTriSaoGai.y;    congBezier.diemQuanTri[3].z = -200.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 880)/45.0f;
      viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
 //     printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
      
      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f; phapThuyenMatDat.y = 1.0f; phapThuyenMatDat.z = 0.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, tocDo, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- chạy qua cầu hướng +z
   else if( soHoatHinh < 1102 ) {
      viTriSaoGai.z = -200.0f + 2.0f*(soHoatHinh - 925);
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, -2.0f, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- giảm tốc độ; quay trở lại nữa
   else if( soHoatHinh < 1147 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;    congBezier.diemQuanTri[0].y = viTriSaoGai.y;    congBezier.diemQuanTri[0].z = 150.0f;
      congBezier.diemQuanTri[1].x = 3.0f;    congBezier.diemQuanTri[1].y = viTriSaoGai.y;    congBezier.diemQuanTri[1].z = 180.0f;
      congBezier.diemQuanTri[2].x = 3.0f;    congBezier.diemQuanTri[2].y = viTriSaoGai.y;    congBezier.diemQuanTri[2].z = 190.0f;
      congBezier.diemQuanTri[3].x = 3.0f;    congBezier.diemQuanTri[3].y = viTriSaoGai.y;    congBezier.diemQuanTri[3].z = 190.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 1102)/45.0f;
      viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
 //     printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
      
      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f; phapThuyenMatDat.y = 1.0f; phapThuyenMatDat.z = 0.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );

      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, tocDo, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- dừng lại lần thứ 2
   else if( soHoatHinh < 1300 ) {
      viTriSaoGai.z = 190.0f;
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, 0.0f, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- bắt đầu chạy qua cầu nữa hướng -z
   else if( soHoatHinh < 1345 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;    congBezier.diemQuanTri[0].y = viTriSaoGai.y;    congBezier.diemQuanTri[0].z = 190.0f;
      congBezier.diemQuanTri[1].x = 3.0f;    congBezier.diemQuanTri[1].y = viTriSaoGai.y;    congBezier.diemQuanTri[1].z = 190.0f;
      congBezier.diemQuanTri[2].x = 3.0f;    congBezier.diemQuanTri[2].y = viTriSaoGai.y;    congBezier.diemQuanTri[2].z = 160.0f;
      congBezier.diemQuanTri[3].x = 3.0f;    congBezier.diemQuanTri[3].y = viTriSaoGai.y;    congBezier.diemQuanTri[3].z = 150.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 1300)/45.0f;
      viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
//      printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
      
      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f; phapThuyenMatDat.y = 1.0f; phapThuyenMatDat.z = 0.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, tocDo, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- chảy qua cầu hướng -z
   else if( soHoatHinh < 1520 ) {
      viTriSaoGai.z = 150.0f - 2.0f*(soHoatHinh - 1345);
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, 2.0f, kSAO_GAI__BAN_KINH*1.4f );
   }
   else if( soHoatHinh < 1565 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;    congBezier.diemQuanTri[0].y = viTriSaoGai.y;    congBezier.diemQuanTri[0].z = -200.0f;
      congBezier.diemQuanTri[1].x = 3.0f;    congBezier.diemQuanTri[1].y = viTriSaoGai.y;    congBezier.diemQuanTri[1].z = -230.0f;
      congBezier.diemQuanTri[2].x = 3.0f;    congBezier.diemQuanTri[2].y = viTriSaoGai.y;    congBezier.diemQuanTri[2].z = -240.0f;
      congBezier.diemQuanTri[3].x = 3.0f;    congBezier.diemQuanTri[3].y = viTriSaoGai.y;    congBezier.diemQuanTri[3].z = -240.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 1520)/45.0f;
      viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
//      printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
      
      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f; phapThuyenMatDat.y = 1.0f; phapThuyenMatDat.z = 0.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, tocDo, kSAO_GAI__BAN_KINH*1.4f );
   }
   // ---- tăng tốc độ; trở lại kiếm
   else if( soHoatHinh < 1610 ) {
      Bezier congBezier;  // cong Bezier cho đoạn thời gian này
      congBezier.diemQuanTri[0].x = 3.0f;    congBezier.diemQuanTri[0].y = viTriSaoGai.y;    congBezier.diemQuanTri[0].z = -240.0f;
      congBezier.diemQuanTri[1].x = 3.0f;    congBezier.diemQuanTri[1].y = viTriSaoGai.y;    congBezier.diemQuanTri[1].z = -240.0f;
      congBezier.diemQuanTri[2].x = 3.0f;    congBezier.diemQuanTri[2].y = viTriSaoGai.y;    congBezier.diemQuanTri[2].z = -230.0f;
      congBezier.diemQuanTri[3].x = 3.0f;    congBezier.diemQuanTri[3].y = viTriSaoGai.y;    congBezier.diemQuanTri[3].z = -200.0f;
      
      float thamSoBezier = ((float)soHoatHinh - 1565)/45.0f;
      viTriSaoGai = tinhViTriBezier3C( &congBezier, thamSoBezier );
      // ---- vận tốc
      Vecto vanToc = tinhVanTocBezier3C( &congBezier, thamSoBezier );
      float tocDo = sqrtf( vanToc.x*vanToc.x + vanToc.z*vanToc.z )/45.0f;
 //     printf( "%d  saoGai tocDo %5.3f\n", soHoatHinh, tocDo );
      
      Vecto phapThuyenMatDat;
      phapThuyenMatDat.x = 0.0f; phapThuyenMatDat.y = 1.0f; phapThuyenMatDat.z = 0.0f;
      trucXoay = tichCoHuong( &phapThuyenMatDat, &vanToc );
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, tocDo, kSAO_GAI__BAN_KINH*1.4f );
   }
   else {
      viTriSaoGai.z = -200.0f + 2.0f*(soHoatHinh - 1610);
      quaternion = tinhXoayChoVatThe( saoGai, trucXoay, -2.0f, kSAO_GAI__BAN_KINH*1.4f );
   }

//   printf( "---- saoGai viTri %5.3f %5.3f %5.3f\n", viTriSaoGai.x, viTriSaoGai.y, viTriSaoGai.z );
//   printf( "   quaternion %5.3f %5.3f %5.3f %5.3f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z );
//   printf( "   truoc saoGai->quaternion %5.3f %5.3f %5.3f %5.3f\n", saoGai->quaternion.w, saoGai->quaternion.x, saoGai->quaternion.y, saoGai->quaternion.z );
   datBienHoaChoVat( saoGai, &phongTo, &quaternion, &viTriSaoGai );
//   printf( "   sau saoGai->quaternion %5.3f %5.3f %5.3f %5.3f\n", saoGai->quaternion.w, saoGai->quaternion.x, saoGai->quaternion.y, saoGai->quaternion.z );
}

void nangCapHinhCauKhongLoXoay( VatThe *hinhCauKhongLoXoay ) {
   
   Vecto trucXoay;
   trucXoay.x = cosf(0.7f);    trucXoay.y = 0.0f;   trucXoay.z = sinf(0.7f);
   Quaternion quaternion = tinhXoayChoVatThe( hinhCauKhongLoXoay, trucXoay, 0.2f, hinhCauKhongLoXoay->hinhDang.hinhCau.banKinh );
//   printf( "hinhCauKhongLoXoay->hinhDang.hinhCau.banKinh %5.3f\n", hinhCauKhongLoXoay->hinhDang.hinhCau.banKinh );
//   printf( "quaternion %5.3f %5.3f %5.3f %5.3f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z );
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   viTri.x = hinhCauKhongLoXoay->dich[12];
   viTri.y = hinhCauKhongLoXoay->dich[13];
   viTri.z = hinhCauKhongLoXoay->dich[14];
   
   datBienHoaChoVat( hinhCauKhongLoXoay, &phongTo, &quaternion, &viTri );
}

void nangCapNapDayHinhTru( VatThe *mangVatThe, unsigned short soLuongVatThe ) {
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   
   Vecto trucXoay;
   trucXoay.x = 0.0f;    trucXoay.y = 0.0f;    trucXoay.z = 1.0f;

   float vanTocGoc = 0.012f;
   Quaternion quaternionXoay = datQuaternionTuVectoVaGocQuay( &trucXoay, vanTocGoc );
   
   unsigned short soVat = 0;
   while ( soVat < soLuongVatThe ) {
      // ---- hai nắp xoay cùng hướng
      VatThe *vatThe = &(mangVatThe[soVat]);
      Vecto viTri;
      viTri.x = vatThe->dich[12];
      viTri.y = vatThe->dich[13];
      viTri.z = vatThe->dich[14];
      Quaternion quaternionVatThe = vatThe->quaternion;
      Quaternion xoayMoi = nhanQuaternionVoiQuaternion( &quaternionVatThe, &quaternionXoay );
      datBienHoaChoVat( vatThe, &phongTo, &xoayMoi, &viTri );
      soVat++;

      // ---- nắp bên kia
      vatThe = &(mangVatThe[soVat]);
      viTri.x = vatThe->dich[12];
      viTri.y = vatThe->dich[13];
      viTri.z = vatThe->dich[14];
      datBienHoaChoVat( vatThe, &phongTo, &xoayMoi, &viTri );
      soVat++;
   }
//   printf("xong nangCap nap day hinh tru\n" );
}

void nangCapBayBongBong( VatThe *danhSachBongBong, unsigned short soLuongBongBong, unsigned short soHoatHinh ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;

   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   Bezier duongBongBong[4];
   duongBongBong[0].diemQuanTri[0].x = 962.0f;   duongBongBong[0].diemQuanTri[0].y = 100.0f;   duongBongBong[0].diemQuanTri[0].z = 10.0f;
   duongBongBong[0].diemQuanTri[1].x = 586.0f;   duongBongBong[0].diemQuanTri[1].y = 100.0f;   duongBongBong[0].diemQuanTri[1].z = -167.0f;
   duongBongBong[0].diemQuanTri[2].x = 552.0f;   duongBongBong[0].diemQuanTri[2].y = 100.0f;   duongBongBong[0].diemQuanTri[2].z = -212.0f;
   duongBongBong[0].diemQuanTri[3].x = 413.0f;   duongBongBong[0].diemQuanTri[3].y = 100.0f;   duongBongBong[0].diemQuanTri[3].z = -218.0f;

   duongBongBong[1].diemQuanTri[0].x = 413.0f;   duongBongBong[1].diemQuanTri[0].y = 100.0f;   duongBongBong[1].diemQuanTri[0].z = -218.0f;
   duongBongBong[1].diemQuanTri[1].x = 275.0f;   duongBongBong[1].diemQuanTri[1].y = 100.0f;   duongBongBong[1].diemQuanTri[1].z = -223.0f;
   duongBongBong[1].diemQuanTri[2].x = 218.0f;   duongBongBong[1].diemQuanTri[2].y = 100.0f;   duongBongBong[1].diemQuanTri[2].z = 36.0f;
   duongBongBong[1].diemQuanTri[3].x = 60.0f;    duongBongBong[1].diemQuanTri[3].y = 100.0f;   duongBongBong[1].diemQuanTri[3].z = 0.0f;
// ----
   duongBongBong[2].diemQuanTri[0].x = 60.0f;     duongBongBong[2].diemQuanTri[0].y = 100.0f;   duongBongBong[2].diemQuanTri[0].z = 0.0f;
   duongBongBong[2].diemQuanTri[1].x = -108.0f;   duongBongBong[2].diemQuanTri[1].y = 100.0f;   duongBongBong[2].diemQuanTri[1].z = -34.0f;
   duongBongBong[2].diemQuanTri[2].x = -270.0f;   duongBongBong[2].diemQuanTri[2].y = 100.0f;   duongBongBong[2].diemQuanTri[2].z = -200.0f;
   duongBongBong[2].diemQuanTri[3].x = -450.0f;   duongBongBong[2].diemQuanTri[3].y = 100.0f;   duongBongBong[2].diemQuanTri[3].z = -259.0f;

   duongBongBong[3].diemQuanTri[0].x = -450.0f;   duongBongBong[3].diemQuanTri[0].y = 100.0f;   duongBongBong[3].diemQuanTri[0].z = -259.0f;
   duongBongBong[3].diemQuanTri[1].x = -632.0f;   duongBongBong[3].diemQuanTri[1].y = 100.0f;   duongBongBong[3].diemQuanTri[1].z = -320.0f;
   duongBongBong[3].diemQuanTri[2].x = -765.0f;   duongBongBong[3].diemQuanTri[2].y = 100.0f;   duongBongBong[3].diemQuanTri[2].z = -82.0f;
   duongBongBong[3].diemQuanTri[3].x = -1034.0f;  duongBongBong[3].diemQuanTri[3].y = 100.0f;   duongBongBong[3].diemQuanTri[3].z = -17.0f;

   // ---- nâng cấp vị trí cho bày bong bóng
   unsigned short soBongBong = 0;
   while ( soBongBong < soLuongBongBong ) {
      
      float thamSoBezier = danhSachBongBong[soBongBong].thamSoBezier + kTOC_DO_BONG_BONG_TRONG_BAY;
      if( thamSoBezier > 4.0f )
         thamSoBezier = 0.0f;

      danhSachBongBong[soBongBong].thamSoBezier = thamSoBezier;
      
      // --- xài tham số Bezier cho tính vị trí mới
      Vecto viTriDiChuyen;
      
      if( thamSoBezier < 1.0f )
         viTriDiChuyen = tinhViTriBezier3C( &(duongBongBong[0]), thamSoBezier );
      else if( thamSoBezier < 2.0f )
         viTriDiChuyen = tinhViTriBezier3C( &(duongBongBong[1]), thamSoBezier - 1.0f );
      else if( thamSoBezier < 3.0f )
         viTriDiChuyen = tinhViTriBezier3C( &(duongBongBong[2]), thamSoBezier - 2.0f );
      else if( thamSoBezier < 4.0f )
         viTriDiChuyen = tinhViTriBezier3C( &(duongBongBong[3]), thamSoBezier - 3.0f );

      // ---- cộng vị trí di chuyển với vị trí ban đầu
      Vecto viTriMoi;
      viTriMoi.x = danhSachBongBong[soBongBong].viTriDau.x + viTriDiChuyen.x;
      viTriMoi.y = danhSachBongBong[soBongBong].viTriDau.y + viTriDiChuyen.y;
      viTriMoi.z = danhSachBongBong[soBongBong].viTriDau.z + viTriDiChuyen.z;
//      printf( "%d %d  viTriBongBong %5.3f %5.3f %5.3f\n", soHoatHinh, soBongBong, viTriMoi.x, viTriMoi.y, viTriMoi.z );

      datBienHoaChoVat( &(danhSachBongBong[soBongBong]), &phongTo, &quaternion, &viTriMoi );
      soBongBong++;
   }
   
}

void nangCapHaiKimTuThapXoay( VatThe *kimTuThap ) {
   
   Vecto trucXoay;
   trucXoay.x = 0.0f;    trucXoay.y = 1.0f;   trucXoay.z = 0.0f;
   Quaternion quaternion = tinhXoayChoVatThe( kimTuThap, trucXoay, 0.5f, 3.0f );

   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ---- kim tư tháp 1
   Vecto viTri;
   viTri.x = kimTuThap->dich[12];
   viTri.y = kimTuThap->dich[13];
   viTri.z = kimTuThap->dich[14];

   datBienHoaChoVat( kimTuThap, &phongTo, &quaternion, &viTri );
   
   // ---- kim tư tháp 2
   trucXoay.y = -1.0f;
   
   viTri.x = kimTuThap[1].dich[12];
   viTri.y = kimTuThap[1].dich[13];
   viTri.z = kimTuThap[1].dich[14];
   datBienHoaChoVat( &(kimTuThap[1]), &phongTo, &quaternion, &viTri );

}

void nangCapHaiThauKinhXoay( VatThe *kimTuThap ) {
   
   Vecto trucXoay;
   trucXoay.x = 0.0f;    trucXoay.y = -1.0f;   trucXoay.z = 0.0f;
   Quaternion quaternion = tinhXoayChoVatThe( kimTuThap, trucXoay, 0.5f, 3.0f );

   // ---- phóng to đặt biệt cho thấu kính
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 0.2f;
   
   // ---- kim tư tháp 1
   Vecto viTri;
   viTri.x = kimTuThap->dich[12];
   viTri.y = kimTuThap->dich[13];
   viTri.z = kimTuThap->dich[14];
   
   datBienHoaChoVat( kimTuThap, &phongTo, &quaternion, &viTri );
   
   // ---- kim tư tháp 2
   trucXoay.y = 1.0f;
   
   viTri.x = kimTuThap[1].dich[12];
   viTri.y = kimTuThap[1].dich[13];
   viTri.z = kimTuThap[1].dich[14];
   datBienHoaChoVat( &(kimTuThap[1]), &phongTo, &quaternion, &viTri );
}


void nangCapVuDinhCong( VatThe *danhSachVat, unsigned short soHoatHinh ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;

   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   // ---- 
   Vecto viTri;
   viTri.x = danhSachVat[0].dich[12];
   viTri.y = 142.0f + 1.5f*sinf( soHoatHinh*0.05f );
   viTri.z = danhSachVat[0].dich[14];
   datBienHoaChoVat( &(danhSachVat[0]), &phongTo, &quaternion, &viTri );
   
   viTri.x = danhSachVat[1].dich[12];
   viTri.y = 146.0f + 1.5f*sinf( soHoatHinh*0.05f + 0.5f );
   viTri.z = danhSachVat[1].dich[14];
   datBienHoaChoVat( &(danhSachVat[1]), &phongTo, &quaternion, &viTri );
   
   viTri.x = danhSachVat[2].dich[12];
   viTri.y = 150.0f + 1.5f*sinf( soHoatHinh*0.05f + 1.0f );
   viTri.z = danhSachVat[2].dich[14];
   datBienHoaChoVat( &(danhSachVat[2]), &phongTo, &quaternion, &viTri );
   
   viTri.x = danhSachVat[3].dich[12];
   viTri.y = 154.0f + 1.5f*sinf( soHoatHinh*0.05f + 1.5f );
   viTri.z = danhSachVat[3].dich[14];
   datBienHoaChoVat( &(danhSachVat[3]), &phongTo, &quaternion, &viTri );
   
   viTri.x = danhSachVat[4].dich[12];
   viTri.y = 158.0f + 1.5f*sinf( soHoatHinh*0.05f + 2.0f );
   viTri.z = danhSachVat[4].dich[14];
   datBienHoaChoVat( &(danhSachVat[4]), &phongTo, &quaternion, &viTri );

   viTri.x = danhSachVat[5].dich[12];
   viTri.y = 162.0f + 1.5f*sinf( soHoatHinh*0.05f + 2.5f );
   viTri.z = danhSachVat[5].dich[14];
   datBienHoaChoVat( &(danhSachVat[5]), &phongTo, &quaternion, &viTri );
}


// f(x) = A*x*sinf( B*t + C*x )
// df/dx = A*sinf( B*t + C*x ) + A*x*C*cosf( B*t + C*x )
void nangCapLaCo( VatThe *danhSachVat, unsigned short soLuongVatThe, unsigned short soHoatHinh ) {

   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Vecto viTri;
   
   float giaTri = soHoatHinh*0.2f;

   unsigned short soMienLaCo = 0;
   while ( soMienLaCo < soLuongVatThe ) {
      viTri = danhSachVat[soMienLaCo].viTriDau;
      // ---- tính quãng từ đấy
      float quangTuDay = viTri.y - 104.0f;
      // ---- tính giá trị sin, sẽ xài cho dịchZ và gócQuay
      float bienDoSin = 0.1f*sinf( giaTri + quangTuDay*0.5f );
      // ---- dịch Z
      viTri.z += quangTuDay*bienDoSin;
   
      // ---- tính góc
      float gocQuay = atanf( 0.1f*bienDoSin + 0.05*quangTuDay*cosf( giaTri + quangTuDay*0.5f ) );
   
      // ---- tính quaternion
      Vecto trucQuay;
      trucQuay.x = 1.0f;  trucQuay.y = 0.0f;  trucQuay.z = 0.0f;
      quaternion = datQuaternionTuVectoVaGocQuay( &trucQuay, gocQuay );
      datBienHoaChoVat( &(danhSachVat[soMienLaCo]), &phongTo, &quaternion, &viTri );
      
      soMienLaCo++;
   }

}


void nangCapLocXoay( VatThe *danhSachVat, unsigned short soHoatHinh ) {
   
   Quaternion quaternion;
   quaternion.w = 1.0f;   quaternion.x = 0.0f;    quaternion.y = 0.0f;    quaternion.z = 0.0f;
   
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   float goc = soHoatHinh*0.3333f;
   float chenhLech = 1.5f*sinf( soHoatHinh*0.5f );
 
   
   unsigned short soVat = 0;
   while ( soVat < kSO_LUONG__HAT_BAY_TRONG_LOC_XOAY ) {

      // ==== bên này
      Vecto viTriDau = danhSachVat[soVat].viTriDau;
      float banKinh = 7.0f + expf( (viTriDau.y - 100.0f)*0.04f ) + chenhLech;  // 100.0f là đố cao mặt đất
      
      // ---- tính vị trí mới
      float banKinhCos = banKinh*cosf( goc );
      float banKinhSin = banKinh*sinf( goc );

      // ---- nâng cấp vị tri
      viTriDau.x += 2.0f;
      viTriDau.z += 0.1f;
      // ---- giữ vị tri mới
      danhSachVat[soVat].viTriDau = viTriDau;
      
      Vecto viTri = viTriDau;
      // ---- 
      viTri.x += banKinhCos;
      viTri.z += banKinhSin;
      datBienHoaChoVat( &(danhSachVat[soVat]), &phongTo, &quaternion, &viTri );
      
      // ==== bên kia
      viTriDau = danhSachVat[soVat+1].viTriDau;
      // ---- nâng cấp vị tri
      viTriDau.x += 2.0f;
      viTriDau.z += 0.1f;
//      printf( "viTriDau %5.3f %5.3f\n", viTriDau.x, viTriDau.z );
      // ---- giữ vị tri mới
      danhSachVat[soVat+1].viTriDau = viTriDau;

      viTri = viTriDau;
      // ----
      viTri.x -= banKinhCos;
      viTri.z -= banKinhSin;
      datBienHoaChoVat( &(danhSachVat[soVat + 1]), &phongTo, &quaternion, &viTri );

      goc += 0.25f;
      soVat +=2;
   }
//   printf( "ViTriLocXoay %5.3f %5.3f %5.3f\n", danhSachVat[0].viTriDau.x, danhSachVat[0].viTriDau.y, danhSachVat[0].viTriDau.z );

}

#pragma mark ---- PHIM TRƯỜNG 2
void chuanBiMayQuayPhimVaMatTroiPhimTruong2( PhimTruong *phimTruong );
unsigned short vatTheThu( VatThe *danhSachVat );

void nangCapOc( VatThe *VatThe, unsigned short soHoatHinh );

PhimTruong datPhimTruongSo2( unsigned int argc, char **argv ) {
   
   PhimTruong phimTruong;
   phimTruong.soNhoiToiDa = 5;
   
   unsigned int soHoatHinhDau = 0;
   unsigned int soHoatHinhCuoi = 100;     // số bức ảnh cuối cho phim trường này
   
   docThamSoHoatHinh( argc, argv, &soHoatHinhDau, &soHoatHinhCuoi );
   if( soHoatHinhDau > 99 )
      soHoatHinhDau = 99;
   if( soHoatHinhCuoi > 100 )     // số bức ảnh cuối cho phim trường này
      soHoatHinhCuoi = 100;
   
   phimTruong.soHoatHinhDau = soHoatHinhDau;
   phimTruong.soHoatHinhHienTai = soHoatHinhDau;
   phimTruong.soHoatHinhCuoi = soHoatHinhCuoi;
   
   phimTruong.soLuongVatThe = 0;
   phimTruong.danhSachVatThe = malloc( kSO_LUONG_VAT_THE_TOI_DA*sizeof(VatThe) );
   
   // ---- chuẩn bị máy quay phim
   chuanBiMayQuayPhimVaMatTroiPhimTruong2( &phimTruong );
   Mau mauDinhTroi;
   mauDinhTroi.d = 0.0f;   mauDinhTroi.l = 0.0f;   mauDinhTroi.x = 0.5f;   mauDinhTroi.dd = 1.0f;
   Mau mauChanTroi;  // chỉ có một;
   mauChanTroi.d = 0.7f;  mauChanTroi.l = 0.7f;   mauChanTroi.x = 1.0f;    mauChanTroi.dd = 1.0f;
   phimTruong.hoaTietBauTroi = datHoaTietBauTroi( &mauDinhTroi, &mauChanTroi, &mauChanTroi, 0.0f );
   
   VatThe *danhSachVat = phimTruong.danhSachVatThe;
   
   // ----
   phimTruong.soLuongVatThe += vatTheThu( &(danhSachVat[phimTruong.soLuongVatThe]) );
   
   return phimTruong;
}

void chuanBiMayQuayPhimVaMatTroiPhimTruong2( PhimTruong *phimTruong ) {
   
   // ==== máy quay phim
   phimTruong->mayQuayPhim.kieuChieu = kKIEU_CHIEU__TOAN_CANH;
   // ---- vị trí bắt đầu cho máy quay phim
   phimTruong->mayQuayPhim.viTri.x = 0.0f;
   phimTruong->mayQuayPhim.viTri.y = 2.6f;
   phimTruong->mayQuayPhim.viTri.z = 10.0f;
   phimTruong->mayQuayPhim.cachManChieu = 5.0f;
   Vecto trucQuayMayQuayPhim;
   trucQuayMayQuayPhim.x = 0.0f;
   trucQuayMayQuayPhim.y = 1.0f;
   trucQuayMayQuayPhim.z = 0.0f;
   
   Quaternion quaternion = datQuaternionTuVectoVaGocQuay( &trucQuayMayQuayPhim, 3.14159f );
   trucQuayMayQuayPhim.x = 1.0f;
   trucQuayMayQuayPhim.y = 0.0f;
   trucQuayMayQuayPhim.z = 0.0f;
   Quaternion quaternion1 = datQuaternionTuVectoVaGocQuay( &trucQuayMayQuayPhim, -0.4f );
   Quaternion quaternionKetQua = nhanQuaternionVoiQuaternion( &quaternion, &quaternion1 );
   phimTruong->mayQuayPhim.quaternion = quaternionKetQua;

   quaternionQuaMaTran( &(phimTruong->mayQuayPhim.quaternion), phimTruong->mayQuayPhim.xoay );
   
   // ---- mặt trời
   Vecto anhSangMatTroi;
   anhSangMatTroi.x = -1.0f;
   anhSangMatTroi.y = -1.0f;
   anhSangMatTroi.z = 0.0f;
   donViHoa( &anhSangMatTroi );
   phimTruong->matTroi.huongAnh = anhSangMatTroi;
   phimTruong->matTroi.mauAnh.d = 1.0f;
   phimTruong->matTroi.mauAnh.l = 1.0f;
   phimTruong->matTroi.mauAnh.x = 1.0f;
   phimTruong->matTroi.mauAnh.dd = 1.0f;
}

unsigned short vatTheThu( VatThe *danhSachVat ) {
   
   Vecto vectoXoay;
   vectoXoay.x = 1.0f;    vectoXoay.y = 0.0f;     vectoXoay.z = 0.0f;
   Quaternion quaternion = datQuaternionTuVectoVaGocQuay( &vectoXoay, 0.0f );
   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;
   
   Mau mau;   Mau mau1;
   Vecto viTri;

   // ----
   mau.d = 1.0f;   mau.l = 0.0f;   mau.x = 0.0f;    mau.dd = 1.0f;    mau.p = 0.3f;
   mau1.d = 1.0f;   mau1.l = 0.0f;   mau1.x = 0.0f;    mau1.dd = 1.0f;    mau1.p = 0.3f;
   
   Mau mauNen;
   Mau mauThap;
   Mau mauCao;
   mauNen.d = 1.0f;
   mauNen.l = 1.0f;
   mauNen.x = 1.0f;
   mauNen.dd = 1.0f;
   mauNen.p = 0.0f;
   mauThap.d = 1.0f;
   mauThap.l = 0.0f;
   mauThap.x = 0.1f;
   mauThap.dd = 1.0f;
   mauThap.p = 0.0f;
   mauCao.d = 1.0f;   mauCao.l = 1.0f;    mauCao.x = 1.0f;   mauCao.dd = 1.0f;   mauCao.p = 0.0f;
   Mau mauOc0;
   Mau mauOc1;
   Mau mauOc2;
   mauOc0.d = 0.0f;     mauOc0.l = 0.3f;      mauOc0.x = 1.0f;     mauOc0.dd = 1.0f;    mauOc0.p = 0.0f;
   mauOc1.d = 0.2f;     mauOc1.l = 0.65f;      mauOc1.x = 1.0f;     mauOc1.dd = 1.0f;    mauOc1.p = 0.0f;
   mauOc2.d = 0.65f;     mauOc2.l = 1.0f;      mauOc2.x = 1.0f;     mauOc2.dd = 1.0f;    mauOc2.p = 0.0f;

   Vecto huongDoc;
   huongDoc.x = 0.0f;   huongDoc.y = 1.0f;   huongDoc.z = 0.0f;
   Vecto huongNgang;
   huongNgang.x = 1.0f;   huongNgang.y = 0.0f;   huongNgang.z = 0.0f;
   unsigned char soLuongVat = 0;
   
   viTri.x = -1.0f;    viTri.y = -2.5f;     viTri.z = 2.0f;
   danhSachVat[soLuongVat].hinhDang.hinhTru = datHinhTru( 4.2f, 1.0f, &(danhSachVat[soLuongVat].baoBiVT));
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mau, &mauOc0, &mauOc1, &mauOc2, 0.1667f, 0.5f, 0.5f, 5.0f, 5.0f, 0.2f, 0.0f, 1.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   soLuongVat++;

   viTri.x = 2.0f;    viTri.y = -2.2f;    viTri.z = 0.0f;
   danhSachVat[soLuongVat].hinhDang.hinhTru = datHinhTru( 4.2f, 1.0f, &(danhSachVat[soLuongVat].baoBiVT));
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HINH_TRU;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietQuanSongTheoHuong = datHoaTietQuanSongTheoHuong( &huongNgang, &huongDoc, &mau, &mauOc0, &mauOc1, &mauOc2, 0.1667f, 0.5f, 0.5f, 5.0f, 5.0f, 0.2f, 0.0f, 1.0f );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__QUAN_SONG_THEO_HUONG;
   soLuongVat++;
   
  
//   viTri.x = 3.0f;    viTri.y = 0.0f;     viTri.z = 0.0f;
//   hoaTietHaiChamBi( &viTri, &(danhSachVat[0].hoaTiet.hoaTietHaiChamBi) );
   mauNen.p = 0.3f;   mauOc0.p = 0.3f;
   viTri.x = -1.0f;    viTri.y = -0.5f;    viTri.z = 1.0f;
   mauNen.dd = 0.2f;
   danhSachVat[soLuongVat].hinhDang.hinhCau = datHinhCau( 0.8f, &(danhSachVat[soLuongVat].baoBiVT));
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong(&mauOc0, &mauNen );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__DI_HUONG;
   soLuongVat++;

//   mauNen.dd = 0.5f;
   viTri.x = 2.5f;    viTri.y = 1.5f;    viTri.z = 0.0f;
   danhSachVat[soLuongVat].hinhDang.hinhCau = datHinhCau( 1.6f, &(danhSachVat[soLuongVat].baoBiVT));
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauOc0, &mauNen );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__DI_HUONG;
   soLuongVat++;

   mauNen.d = 1.0f;   mauNen.l = 0.0f;   mauNen.x = 0.0f;  mauNen.dd = 1.0f;
   mauOc0.d = 1.0f;   mauOc0.l = 0.0f;   mauOc0.x = 0.5f;  mauOc0.dd = 1.0f;
   viTri.x = -3.0f;
   viTri.z = 2.0f;
   viTri.y = -1.0f;
   danhSachVat[soLuongVat].hinhDang.thapNhiDien = datThapNhiDien( 1.0f, 1.0f, 1.0f, &(danhSachVat[soLuongVat].baoBiVT));
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__THAP_NHI_DIEN;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauOc0, &mauNen );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__DI_HUONG;
   soLuongVat++;
   
   mauNen.d = 0.5f;   mauNen.l = 0.0f;   mauNen.x = 1.0f;  mauNen.dd = 1.0f;
   mauOc0.d = 0.3f;   mauOc0.l = 0.0f;   mauOc0.x = 1.0f;  mauOc0.dd = 1.0f;
   viTri.x = -2.5f;
   viTri.z = 4.5f;
   viTri.y = -1.0f;
   danhSachVat[soLuongVat].hinhDang.thapNhiDien = datThapNhiDien( 1.0f, 1.0f, 1.0f, &(danhSachVat[soLuongVat].baoBiVT));
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__THAP_NHI_DIEN;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauOc0, &mauNen );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__DI_HUONG;
   soLuongVat++;
   
   mauNen.d = 0.0f;   mauNen.l = 0.0f;   mauNen.x = 1.0f;  mauNen.dd = 1.0f;
   mauOc0.d = 0.0f;   mauOc0.l = 0.0f;   mauOc0.x = 1.0f;  mauOc0.dd = 1.0f;
   viTri.x = 2.5f;
   viTri.z = 2.5f;
   viTri.y = -1.0f;
   danhSachVat[soLuongVat].hinhDang.thapNhiDien = datThapNhiDien( 1.0f, 1.0f, 1.0f, &(danhSachVat[soLuongVat].baoBiVT));
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__THAP_NHI_DIEN;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietDiHuong = datHoaTietDiHuong( &mauOc0, &mauNen );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__DI_HUONG;
   soLuongVat++;

   viTri.x = 0.0f; viTri.y += 1.5f;   viTri.z = 3.0f;
   danhSachVat[soLuongVat].hinhDang.hinhCau = datHinhCau( 1.2f, &(danhSachVat[soLuongVat].baoBiVT));
   danhSachVat[soLuongVat].loai = kLOAI_HINH_DANG__HINH_CAU;
   danhSachVat[soLuongVat].chietSuat = 1.0f;
   datBienHoaChoVat( &(danhSachVat[soLuongVat]), &phongTo, &quaternion, &viTri );
   danhSachVat[soLuongVat].hoaTiet.hoaTietNgoiSaoCau = datHoaTietNgoiSaoCau( &mauNen, &mauNen, &mauOc2, 0.2f, 0.1f, 0.0f, 5 );
   danhSachVat[soLuongVat].soHoaTiet = kHOA_TIET__NGOI_SAO_CAU;
   soLuongVat++;

   return soLuongVat;
}

#pragma mark ---- NÂNG CẤP PHIM TRƯỜNG 2
void nangCapPhimTruong2( PhimTruong *phimTruong ) {
   
   //   printf( "phimTruong->soHoatHinhDau; %d\n", phimTruong->soHoatHinhDau );
//   nangCapPhimTruong0_mayQuayPhim( phimTruong );
//   nangCapPhimTruong0_nhanVat( phimTruong, phimTruong->soHoatHinhDau );
   nangCapOc( phimTruong->danhSachVatThe, phimTruong->soHoatHinhDau );
   
   // ---- tăng số hoạt hình
   phimTruong->soHoatHinhDau++;
}

void nangCapOc( VatThe *vatThe, unsigned short soHoatHinh ) {
   
   Vecto trucXoay;
   trucXoay.x = 0.0f;    trucXoay.y = 1.0f;     trucXoay.z = 0.0f;

   Vecto phongTo;
   phongTo.x = 1.0f;     phongTo.y = 1.0f;     phongTo.z = 1.0f;

   
}
