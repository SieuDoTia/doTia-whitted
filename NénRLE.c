// Nếu bạn muốn xài phương pháp nén RLE
// Tôi không xài này nữa ngưng muốn giữ lại nếu cần xài trong tương lại

// ---- cho tệp đầu h
void luuAnhRLE( char *tenTep, Anh *anh, unsigned char kieuDuLieu, unsigned short thoiGianKetXuat );   // lưu ảnh RLE

// ---- Cho lưu tệp
void luuThongTinKenh_EXR( FILE *tep, unsigned char *danhSachKenh, unsigned char soLuongKenh, unsigned char kieuDuLieu );
void luuThongTinCuaSoDuLieu( FILE *tep, unsigned int beRong, unsigned int beCao );
void luuThongTinCuaSoChieu( FILE *tep, unsigned int beRong, unsigned int beCao );
void luuThoiGianKetXuat( FILE *tep, unsigned short thoiGianKetXuat );
void luuBangDuLieuAnh( FILE *tep, unsigned short beRong, unsigned short beCao, unsigned char soLuongKenh, unsigned char kieuDuLieu );
void chepDuLieuKenhFloat( unsigned char *dem, const float *kenh, unsigned short beRong );
void chepDuLieuKenhHalf( unsigned char *dem, const float *kenh, unsigned short beRong );
unsigned short doiFloatSangHalf( float soFloat );
void locDuLieuTrongDem(unsigned char *dem, unsigned int beDai, unsigned char *demLoc);
unsigned int nenRLE(unsigned char *dem, int beDai, unsigned char *demNen);

#pragma mark ---- Lưư Ảnh

/* Lưu Ảnh RLE */
void luuAnhRLE( char *tenTep, Anh *anh, unsigned char kieuDuLieu, unsigned short thoiGianKetXuat ) {
   
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

   fputc( 0x01, tep );  // RLE

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

   fputc( 0x00, tep );   // 1.0
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
   unsigned long long *bangThanhPhan = malloc( beCao << 3 );
   
   // ---- bề dài dệm
   unsigned int beDaiDem = (beRong << kieuDuLieu)*3; // nhân 3 cho 3 kênh 
   // ---- tạo đệm để lọc dữ liệu
   unsigned char *dem = malloc( beDaiDem );
   unsigned char *demLoc = malloc( beDaiDem );
   unsigned char *demNenRLE = malloc( beDaiDem << 1);  // nhân 2 cho an toàn
   
   // ---- lưu dữ liệu cho thành phần ảnh
   unsigned short soHang = 0;
   while( soHang < beCao ) {
      
      bangThanhPhan[soHang] = ftell( tep );

      // ---- luư số hàng
      fputc( soHang & 0xff, tep );
      fputc( (soHang >> 8), tep );
      fputc( (soHang >> 16), tep );
      fputc( (soHang >> 24), tep );
      
      // ---- dữ liệu kênh
      unsigned int diaChiKenhBatDau = beRong*(beCao - soHang - 1);
      unsigned int diaChiDem = 0;

      if( kieuDuLieu == kKIEU_FLOAT ) {
         // ---- chép kênh xanh
         chepDuLieuKenhFloat( dem, &(anh->kenhXanh[diaChiKenhBatDau]), beRong);
         // ---- chép kênh lục
         diaChiDem += beRong << kieuDuLieu;
         chepDuLieuKenhFloat( &(dem[diaChiDem]), &(anh->kenhLuc[diaChiKenhBatDau]), beRong);
         // ---- chép kênh đỏ
         diaChiDem += beRong << kieuDuLieu;
         chepDuLieuKenhFloat( &(dem[diaChiDem]), &(anh->kenhDo[diaChiKenhBatDau]), beRong);
      }
      else {
         // ---- chép kênh xanh
         chepDuLieuKenhHalf( dem, &(anh->kenhXanh[diaChiKenhBatDau]), beRong);
         // ---- chép kênh lục
         diaChiDem += beRong << kieuDuLieu;
         chepDuLieuKenhHalf( &(dem[diaChiDem]), &(anh->kenhLuc[diaChiKenhBatDau]), beRong);
         // ---- chép kênh đỏ
         diaChiDem += beRong << kieuDuLieu;
         chepDuLieuKenhHalf( &(dem[diaChiDem]), &(anh->kenhDo[diaChiKenhBatDau]), beRong);
      }

      locDuLieuTrongDem( dem, beDaiDem, demLoc);
      unsigned int beDaiDuLieuNen = nenRLE( demLoc, beDaiDem, demNenRLE );
      
      fputc( beDaiDuLieuNen & 0xff, tep );
      fputc( (beDaiDuLieuNen >> 8), tep );
      fputc( (beDaiDuLieuNen >> 16), tep );
      fputc( (beDaiDuLieuNen >> 24), tep );
      
      // ---- lưu dữ liệu nén
      unsigned int diaChi = 0;
      while( diaChi < beDaiDuLieuNen ) {
         fputc( demNenRLE[diaChi], tep );
         diaChi++;
      }

      soHang++;
   }
   
   // ---- lưu bảng thành phân
   fseek( tep, diaChiDauBangThanhPhan, SEEK_SET );
   soHang = 0;
   while( soHang < beCao ) {
      unsigned long long diaChiThanhPhan = bangThanhPhan[soHang];
      fputc( diaChiThanhPhan & 0xff, tep );
      fputc( (diaChiThanhPhan >> 8), tep );
      fputc( (diaChiThanhPhan >> 16), tep );
      fputc( (diaChiThanhPhan >> 24), tep );
      fputc( (diaChiThanhPhan >> 32), tep );
      fputc( (diaChiThanhPhan >> 40), tep );
      fputc( (diaChiThanhPhan >> 48), tep );
      fputc( (diaChiThanhPhan >> 56), tep );
      soHang++;
   }
 
   // ---- thả trí nhớ
   free( dem );
   free( demLoc );
   free( demNenRLE );
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

#define kDAY_TOI_THIEU 3  // dãy tối thiếu
#define kDAY_TOI_DA 127   // dãy tối đa
// ---- Từ thư viện OpenEXR
unsigned int nenRLE(unsigned char *dem, int beDai, unsigned char *demNen) {

   const unsigned char *demKetThuc = dem + beDai; // kết thúc của đệm
   const unsigned char *dayBatDau = dem;  // dãy bắt đầu
   const unsigned char *dayKetThuc = dem + 1; // dãy kết thúc
   unsigned char *diaChiDemNen = demNen;  // địa chỉ trong đệm nén
   
   // ---- khi chưa tới két thúc đệm
   while (dayBatDau < demKetThuc) {
      
      // ---- đếm số lượng byte cùng giá trị; cẩn thận đừng về kết thúc đệm, dãy lớn hơnkDAY_TOI_DA
      while (dayKetThuc < demKetThuc && *dayBatDau == *dayKetThuc && dayKetThuc - dayBatDau - 1 <kDAY_TOI_DA) {
         ++dayKetThuc;
      }
      // ---- nếu số lượng byte giống >= kDAY_TOI_THIEU
      if (dayKetThuc - dayBatDau >= kDAY_TOI_THIEU) {
         //
         // có thể nén
         //
         // ---- số lượng byte cùng giá trị - 1
         *diaChiDemNen++ = (dayKetThuc - dayBatDau) - 1;
         // ---- giá trị byte
         *diaChiDemNen++ = *(signed char *) dayBatDau;
         // ---- move to where different value found orkDAY_TOI_DA (smallest of these two)
         dayBatDau = dayKetThuc;
      }
      else {
         //
         // có dãy byte khác nhau
         //
         // ---- đếm số lượng byte không giống; cần thận về kết thúc đệm,
         while (dayKetThuc < demKetThuc &&
                ((dayKetThuc + 1 >= demKetThuc || *dayKetThuc != *(dayKetThuc + 1)) || (dayKetThuc + 2 >= demKetThuc || *(dayKetThuc + 1) != *(dayKetThuc + 2))) &&
                dayKetThuc - dayBatDau <kDAY_TOI_DA) {
            ++dayKetThuc;
         }
         // ---- số lượng byte không giống
         *diaChiDemNen++ = dayBatDau - dayKetThuc;
         // ---- lưu byte không giống
         while (dayBatDau < dayKetThuc) {
            *diaChiDemNen++ = *(signed char *) (dayBatDau++);
         }
      }
      // ---- move to next byte
      ++dayKetThuc;
   }
   
   return (unsigned int)(diaChiDemNen - demNen);
}
