// Nếu bạn muốn xài phương pháp không nén
// Tôi không xài này nữa ngưng muốn giữ lại nếu cần xài trong tương lại

// ---- cho tệp đầu h
void luuAnh( char *tenTep, Anh *anh, unsigned char kieuDuLieu, unsigned short thoiGianKetXuat );

// ---- Cho lưu tệp
void luuThongTinKenh_EXR( FILE *tep, unsigned char *danhSachKenh, unsigned char soLuongKenh, unsigned char kieuDuLieu );
void luuThongTinCuaSoDuLieu( FILE *tep, unsigned int beRong, unsigned int beCao );
void luuThongTinCuaSoChieu( FILE *tep, unsigned int beRong, unsigned int beCao );
void luuThoiGianKetXuat( FILE *tep, unsigned short thoiGianKetXuat );
void luuBangDuLieuAnh( FILE *tep, unsigned short beRong, unsigned short beCao, unsigned char soLuongKenh, unsigned char kieuDuLieu );
void luuDuLieuKenhFloat( FILE *tep, const float *kenh, unsigned int diaChi, unsigned short beRong );
void luuDuLieuKenhHalf( FILE *tep, const float *kenh, unsigned int diaChi, unsigned short beRong );
unsigned short doiFloatSangHalf( float soFloat );

void luuAnh( char *tenTep, Anh *anh, unsigned char kieuDuLieu, unsigned short thoiGianKetXuat ) {
   
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

   fputc( 0x00, tep );  // không nén

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
   luuBangDuLieuAnh( tep, beRong, beCao, 3, kieuDuLieu );
   
   // ---- lưu dữ liệu cho thành phần ảnh
   unsigned short soHang = 0;
   while( soHang < beCao ) {

      // ---- luư số hàng
      fputc( soHang & 0xff, tep );
      fputc( (soHang >> 8), tep );
      fputc( (soHang >> 16), tep );
      fputc( (soHang >> 24), tep );
      
      // ---- bề dài dữ liệu
      unsigned int beDauDuLieu = beRong*3 << kieuDuLieu;  // 1 cho half, 2 cho float
      fputc( beDauDuLieu & 0xff, tep );
      fputc( (beDauDuLieu >> 8), tep );
      fputc( (beDauDuLieu >> 16), tep );
      fputc( (beDauDuLieu >> 24), tep );
      
      // ---- dữ liệu kênh
      unsigned int diaChi = beRong*(beCao - soHang - 1);
      if( kieuDuLieu == kKIEU_FLOAT ) {
         luuDuLieuKenhFloat( tep, anh->kenhXanh, diaChi, beRong );
         luuDuLieuKenhFloat( tep, anh->kenhLuc, diaChi, beRong );
         luuDuLieuKenhFloat( tep, anh->kenhDo, diaChi, beRong );
      }
      else {
         luuDuLieuKenhHalf( tep, anh->kenhXanh, diaChi, beRong );
         luuDuLieuKenhHalf( tep, anh->kenhLuc, diaChi, beRong );
         luuDuLieuKenhHalf( tep, anh->kenhDo, diaChi, beRong );
      }

      soHang++;
   }
 
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

void luuDuLieuKenhFloat( FILE *tep, const float *kenh, unsigned int diaChi, unsigned short beRong ) {

   unsigned short soCot = 0;
   while( soCot < beRong ) {

      union {
         float f;
         unsigned int i;
      } ui;
      
      ui.f = kenh[diaChi + soCot];
      fputc( ui.i & 0xff, tep );
      fputc( (ui.i >> 8), tep );
      fputc( (ui.i >> 16), tep );
      fputc( (ui.i >> 24), tep );
      soCot++;
   }
   // ---- kênh tiếp
}

void luuDuLieuKenhHalf( FILE *tep, const float *kenh, unsigned int diaChi, unsigned short beRong ) {
   
   unsigned short soCot = 0;
   while( soCot < beRong ) {

      unsigned short h = doiFloatSangHalf( kenh[diaChi + soCot] );
      fputc( h & 0xff, tep );
      fputc( (h >> 8), tep );
      soCot++;
   }
   // ---- kênh tiếp
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
