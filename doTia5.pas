{PASCAL: Chương Trình Dò Tia - Phiên bản 0.5 - 2015-05-08}
{Lưu tệp dạnh ảnh EXR}

PROGRAM doTia;

USES CRT;

{hằng số}
CONST
   {phân giải ảnh}
   kBE_RONG = 512;   {bề rộng ảnh, số lượng điểm ảnh}
   kBE_CAO = 512;    {bề cao ảnh, số lượng điểm ảnh}

   kVO_CUC = 10000000; {vô cực}
   kNHOI_TOI_DA = 5;   {lần nhồi tia phản xạ}



{bản ghi màu}
TYPE
    Mau=RECORD
        d:SINGLE;   {đỏ}
        l:SINGLE;   {lục}
        x:SINGLE;   {xanh}
        dd:SINGLE;  {độ đục}
    END;

{bản ghi vecto}
TYPE
    Vecto=RECORD
        x:SINGLE;   {thành phần vectơ}
        y:SINGLE;
        z:SINGLE;
    END;

{bản ghi tia}
TYPE
    Tia=RECORD
        goc:Vecto;   {gốc tia}
        huong:Vecto; {hướng tia}
    END;

{bản ghi cho mảng điểm ảnh, mảng của màu}
TYPE
    MangDiemAnh = ARRAY[1..kBE_RONG*kBE_CAO] OF Mau;


{bản ghi hình cầu}
TYPE
   HinhCau=RECORD
       tam:Vecto;    {vectơ}
       banKinh:SINGLE; {bán kính}
       mauTo:Mau;    {màu tô}
       chietSuat:SINGLE; {chiết suất}
       tiSoPhanXa:SINGLE;   {tỉ số phản xạ}
   END;

{bản ghi mặt phẳng}
TYPE
   MatPhang=RECORD
       tam:Vecto;    {vectơ}
       beRong:SINGLE;  {bề rộng}
       beCao:SINGLE;   {bề cao}
       mauTo:Mau;    {màu tô}
       tiSoPhanXa:SINGLE;   {tỉ số phản xạ}
   END;

{bản ghi thông tin trúng vật thể}
TYPE
   ThongTinTrung=RECORD
      diemTrung:Vecto; {điểm trúng trên mặt vật thể}
      phapTuyen:Vecto; {pháp tuyến tại điểm trúng trên mặt vật thể}
      cachXa:SINGLE;     {tham số t, cách xa tương đối từ góc tia}
      mauTo:Mau;      {màu của vật thể trúng}
      chietSuat:SINGLE; {chiết suất của vật thể trúng}
      tiSoPhanXa:SINGLE;     {tỉ số phản xạ}
      huongTiaTrung:Vecto;
   END;

{bản ghi cho mảng điểm ảnh}
TYPE
    KenhAnh = ARRAY[0..kBE_RONG*kBE_CAO - 1] OF SINGLE;


{bản ghi cho thông tin ảnh}
TYPE
   Anh=RECORD
      beRong:INTEGER;   // bề rộng
      beCao:INTEGER;    // bề cao
      kenhDo:KenhAnh;      // kênh đỏ
      kenhLuc:KenhAnh;     // kênh lục
      kenhXanh:KenhAnh;    // kênh xanh
   END;

{bản ghi cho tên cac kênh ảnh}
TYPE
    MangTenKenh = ARRAY[0..3] OF BYTE;


{bản ghi cho đổi kiểu SINGLE thành mảng BYTE cho lưu trong tệp}
TYPE
   MangByte = ARRAY[0..3] of BYTE;


TYPE
   TapHop=RECORD
      CASE CHAR OF
         's':(soThat:SINGLE);   {kiểu SINGLE}
         'b':(dayByte:MangByte); {kiểu mảng byte}
   END;


TYPE
   duLieuTepAnh = FILE OF BYTE;


{hàm đơn vị hóa vectơ, độ lớn = 1}
FUNCTION donViHoa( vectoDonViHoa:Vecto ): Vecto;
   VAR
      doLon:SINGLE; {độ lớn của vectơ}
      BEGIN
         doLon := vectoDonViHoa.x*vectoDonViHoa.x + vectoDonViHoa.y*vectoDonViHoa.y + vectoDonViHoa.z*vectoDonViHoa.z;
   IF doLon > 0.0 THEN
      BEGIN
         doLon := SQRT( doLon );
         vectoDonViHoa.x := vectoDonViHoa.x/doLon;
         vectoDonViHoa.y := vectoDonViHoa.y/doLon;
         vectoDonViHoa.z := vectoDonViHoa.z/doLon;
      END;
      donViHoa := vectoDonViHoa;
   END;


{hàm tạo hình cầu}
FUNCTION taoHinhCau( tam:Vecto; banKinh:SINGLE; mauTo:Mau; chietSuat, tiSoPhanXa:SINGLE): HinhCau;
   VAR
      hinhCauMoi:HinhCau; {hình cầu mới}
   BEGIN
     hinhCauMoi.tam := tam;
     hinhCauMoi.banKinh := banKinh;
     hinhCauMoi.mauTo := mauTo;
     hinhCauMoi.chietSuat := chietSuat;
     hinhCauMoi.tiSoPhanXa := tiSoPhanXa;
     taoHinhCau := hinhCauMoi;
   END;

{hàm tạo mặt phẳng}
FUNCTION taoMatPhang( tam:Vecto; beRong, beCao:SINGLE; mauTo:Mau; tiSoPhanXa:SINGLE): MatPhang;
   VAR
      matPhangMoi:MatPhang; {mặt phẳng mới}
   BEGIN
     matPhangMoi.tam := tam;
     matPhangMoi.beRong := beRong;
     matPhangMoi.beCao := beCao;
     matPhangMoi.mauTo := mauTo;
     matPhangMoi.tiSoPhanXa := tiSoPhanXa;
     taoMatPhang := matPhangMoi;
   END;


FUNCTION xemCatHinhCau( hinhCauXem:HinhCau; tiaDo:Tia ): ThongTinTrung;

VAR
   huongDenHinhCau:Vecto; {hướng đến tâm hình cầu}
   A:SINGLE;  {a của công thức bậc hai}
   B:SINGLE;  {b của công thức bậc hai}
   C:SINGLE;  {c của công thức bậc hai}
   D:SINGLE;  {biệt thức của công thức bậc hai}
   nghiem0:SINGLE; {nghiệm 0 từ công thức bậc hai}
   nghiem1:SINGLE; {nghiệm 1 từ công thức bậc hai}
   thongTin:ThongTinTrung; {thông tin cho vật thể này}

   BEGIN
   {đặt không trúng}
   thongTin.cachXa := kVO_CUC;

   {tính vectơ từ trung tâm hình cầu đến điểm nhìn}
   huongDenHinhCau.x := tiaDo.goc.x - hinhCauXem.tam.x;
   huongDenHinhCau.y := tiaDo.goc.y - hinhCauXem.tam.y;
   huongDenHinhCau.z := tiaDo.goc.z - hinhCauXem.tam.z;

   A := tiaDo.huong.x*tiaDo.huong.x + tiaDo.huong.y*tiaDo.huong.y + tiaDo.huong.z*tiaDo.huong.z;
   B := 2.0*(tiaDo.huong.x*huongDenHinhCau.x + tiaDo.huong.y*huongDenHinhCau.y + tiaDo.huong.z*huongDenHinhCau.z);
   C := huongDenHinhCau.x*huongDenHinhCau.x + huongDenHinhCau.y*huongDenHinhCau.y + huongDenHinhCau.z*huongDenHinhCau.z - hinhCauXem.banKinh*hinhCauXem.banKinh;
   D := B*B - 4.0*A*C;

   IF D > 0.0 THEN
      BEGIN
      {tính nghiệm}
      nghiem0 := (-B + SQRT(D))/(2.0*A);
      nghiem1 := (-B - SQRT(D))/(2.0*A);

      {tránh vất đề tròn số}
      IF nghiem0 < 0.001 THEN
         nghiem0 := kVO_CUC;
      IF nghiem1 < 0.001 THEN
         nghiem1 := kVO_CUC;

      {tìm nghiệm gần nhất}
      IF nghiem0 < nghiem1 THEN
         thongTin.cachXa := nghiem0
      ELSE
         thongTin.cachXa := nghiem1;
    END;

   IF thongTin.cachXa < kVO_CUC THEN
      BEGIN
         {tính điểm trúng}
         thongTin.diemTrung.x := tiaDo.goc.x + thongTin.cachXa*tiaDo.huong.x;
         thongTin.diemTrung.y := tiaDo.goc.y + thongTin.cachXa*tiaDo.huong.y;
         thongTin.diemTrung.z := tiaDo.goc.z + thongTin.cachXa*tiaDo.huong.z;
         {tính pháp tuyến}
         thongTin.phapTuyen.x := thongTin.diemTrung.x - hinhCauXem.tam.x;
         thongTin.phapTuyen.y := thongTin.diemTrung.y - hinhCauXem.tam.y;
         thongTin.phapTuyen.z := thongTin.diemTrung.z - hinhCauXem.tam.z;

         thongTin.phapTuyen := donViHoa( thongTin.phapTuyen );

         {chép thông tin tô màu}
         thongTin.mauTo := hinhCauXem.mauTo;
         thongTin.chietSuat := hinhCauXem.chietSuat;
         thongTin.tiSoPhanXa := hinhCauXem.tiSoPhanXa;

         {chép hướng trúng}
         thongTin.huongTiaTrung := tiaDo.huong;
   END;

    xemCatHinhCau := thongTin;
END;


{xem tia cắt mặt phẳng vuông góc trục z}
FUNCTION xemCatMatPhang( matPhangXem:MatPhang; tiaDo:Tia ): ThongTinTrung;

VAR
   thongTin:ThongTinTrung;
   toaDoX:SINGLE; {tọa độ x điểm trúng trong mặt phẳng z = tâm.z}
   toaDOY:SINGLE; {tọa độ y điểm trúng trong mặt phẳng z = tâm.z}

BEGIN
   {đặt không trúng}
   thongTin.cachXa := kVO_CUC;

   {nếu tia không songๆ với mặt phẳng có thể trúng nó}
   IF tiaDo.huong.z <> 0.0 THEN
      BEGIN

      {tính cách xa}
      thongTin.cachXa := (matPhangXem.tam.z - tiaDo.goc.z)/tiaDo.huong.z;
      IF thongTin.cachXa > 0.01 THEN
         BEGIN

         {xem hướng x}
         toaDoX := tiaDo.goc.x + thongTin.cachXa*tiaDo.huong.x;

         IF (toaDoX > matPhangXem.tam.x - matPhangXem.beRong*0.5) AND
            (toaDoX < matPhangXem.tam.x + matPhangXem.beRong*0.5) THEN
            BEGIN

            {xem hướng y}
            toaDoY := tiaDo.goc.y + thongTin.cachXa*tiaDo.huong.y;

            IF (toaDoY > matPhangXem.tam.y - matPhangXem.beCao*0.5) AND
               (toaDoY < matPhangXem.tam.y + matPhangXem.beCao*0.5) THEN
               BEGIN

               {đặt pháp tuyến}
               IF tiaDo.huong.z < 0.0 THEN // trúng phía trước
                  BEGIN
                     thongTin.phapTuyen.x := 0.0;
                     thongTin.phapTuyen.y := 0.0;
                     thongTin.phapTuyen.z := 1.0;
                  END
               ELSE // trúng ở phía sau
                  BEGIN
                     thongTin.phapTuyen.x := 0.0;
                     thongTin.phapTuyen.y := 0.0;
                     thongTin.phapTuyen.z := -1.0;
                  END;

               {tính điểm trúng}
               thongTin.diemTrung.x := tiaDo.goc.x + thongTin.cachXa*tiaDo.huong.x;
               thongTin.diemTrung.y := tiaDo.goc.y + thongTin.cachXa*tiaDo.huong.y;
               thongTin.diemTrung.z := tiaDo.goc.z + thongTin.cachXa*tiaDo.huong.z;

               {chép thông tin tô màu}
               thongTin.mauTo := matPhangXem.mauTo;
               thongTin.tiSoPhanXa := matPhangXem.tiSoPhanXa;

               {chép hướng trúng}
               thongTin.huongTiaTrung := tiaDo.huong;
              END
            ELSE {toạ độ y ở ngoài đoạn mặt phẳng}
               thongTin.cachXa := kVO_CUC;
           END
         ELSE {toạ độ x ở ngoài đoạn mặt phẳng}
            thongTin.cachXa := kVO_CUC;

         END
      ELSE {không trúng vì mặt phẳng ở đang sau góc tia}
         thongTin.cachXa := kVO_CUC;
   END;

    xemCatMatPhang := thongTin;
END;


{tính màu tán xạ}
FUNCTION tinhMauTanXa( mauAnhSang, mauVat:Mau; huongNguonAnhSang, phapTuyen:Vecto ): Mau;

VAR
   tichVoHuong:SINGLE; {tích vô hướng pháp tuyến với hướng nguồn ánh sáng}
   mauTanXa:Mau;  {màu khuếch xạ từ vật thể}

BEGIN
   {tính độ sáng}
   tichVoHuong := huongNguonAnhSang.x*phapTuyen.x + huongNguonAnhSang.y*phapTuyen.y + huongNguonAnhSang.z*phapTuyen.z;

   {ánh sáng trúng mặt có hướng nghịch chiếu với pháp tuyến}
   tichVoHuong := -tichVoHuong;

   IF tichVoHuong < 0.0 THEN
      tichVoHuong := 0.0;  {cho một chút ánh sáng để giả bộ ánh sáng từ môi trường}

   {nhân độ sáng với màu vật thể}
   mauTanXa.d := tichVoHuong*(mauVat.d);
   mauTanXa.l := tichVoHuong*(mauVat.l);
   mauTanXa.x := tichVoHuong*(mauVat.x);
   mauTanXa.dd := 1.0;

   {chỉnh màu theo màu ánh sáng nguồn}
   mauTanXa.d := mauTanXa.d*mauAnhSang.d;
   mauTanXa.l := mauTanXa.l*mauAnhSang.l;
   mauTanXa.x := mauTanXa.x*mauAnhSang.x;

   {cho bóng không qúa tối}
   mauTanXa.d := mauTanXa.d + 0.05*mauVat.d;
   mauTanXa.l := mauTanXa.l + 0.05*mauVat.l;
   mauTanXa.x := mauTanXa.x + 0.05*mauVat.x;

   tinhMauTanXa := mauTanXa;
END;


{tính tia phản xạ}
FUNCTION tinhTiaPhanXa( phapTuyen, diemTrung, huongTrung:Vecto ): Tia;

VAR
   tichVoHuong:SINGLE; {tích vô hướng pháp tuyến với hướng tia trúng}
   tiaPhanXa:Tia;  {tia phản xạ}

BEGIN
   {tích vô hướng giữa hướng tia gốc với vectơ vuông góc}
   huongTrung := donViHoa( huongTrung );
   tichVoHuong := phapTuyen.x*huongTrung.x + phapTuyen.y*huongTrung.y + phapTuyen.z*huongTrung.z;

   {tính hướng tia phản xạ}
   tiaPhanXa.huong.x := huongTrung.x - 2.0*tichVoHuong*phapTuyen.x;
   tiaPhanXa.huong.y := huongTrung.y - 2.0*tichVoHuong*phapTuyen.y;
   tiaPhanXa.huong.z := huongTrung.z - 2.0*tichVoHuong*phapTuyen.z;

   {cộng thêm một chút cho ra khỏi vật thể}
   tiaPhanXa.goc.x := diemTrung.x + 0.001*tiaPhanXa.huong.x;
   tiaPhanXa.goc.y := diemTrung.y + 0.001*tiaPhanXa.huong.y;
   tiaPhanXa.goc.z := diemTrung.z + 0.001*tiaPhanXa.huong.z;

   tinhTiaPhanXa := tiaPhanXa;
END;


{tính tia khúc xạ}
FUNCTION tinhTiaKhucXa( phapTuyen, diemTrung, huongTrung:Vecto; chietSuat:SINGLE ): Tia;

VAR
   tichVoHuong:SINGLE;  {tích vô hướng pháp tuyến với hướng tia trúng}
   tiaKhucXa:Tia;  {tia khúc xạ}
   tiSoXuyen:SINGLE; {tỉ số chiết suất, giả sư ở vật thể là không khí}
   a:SINGLE;        {biến xài tính hướng khúc xạ}
   b:SINGLE;        {biến xài tính hướng khúc xạ}

BEGIN
   {tia trúng cùng hướng với pháp tuyến cho biết hướng khúc xạ}
   donViHoa( &huongTrung );
   tichVoHuong := phapTuyen.x*huongTrung.x + phapTuyen.y*huongTrung.y + phapTuyen.z*huongTrung.z;

   IF tichVoHuong < 0.0 THEN {vào vật thể}
      BEGIN
         tiSoXuyen := 1.0/chietSuat; {sin θ2 - tỉ số chiết suất}
         a := 1.0 - tichVoHuong*tichVoHuong;   // nhớ tích vô hướng < 0.0
         b := SQRT( 1.0 - tiSoXuyen*tiSoXuyen*a );  // cos θ2

         tiaKhucXa.huong.x := (huongTrung.x - phapTuyen.x*tichVoHuong)*tiSoXuyen - phapTuyen.x*b;
         tiaKhucXa.huong.y := (huongTrung.y - phapTuyen.y*tichVoHuong)*tiSoXuyen - phapTuyen.y*b;
         tiaKhucXa.huong.z := (huongTrung.z - phapTuyen.z*tichVoHuong)*tiSoXuyen - phapTuyen.z*b;

         {cộng thêm một chút cho vào vật thể}
         tiaKhucXa.goc.x := diemTrung.x + 0.001*tiaKhucXa.huong.x;
         tiaKhucXa.goc.y := diemTrung.y + 0.001*tiaKhucXa.huong.y;
         tiaKhucXa.goc.z := diemTrung.z + 0.001*tiaKhucXa.huong.z;
      END

   ELSE {ra vật thể}
      BEGIN
         tiSoXuyen := chietSuat/1.0;           {sin θ2 - tỉ số chiết suất}
         a := 1.0 - tichVoHuong*tichVoHuong;   {nhớ tích vô hướng < 0.0}
         b := 1.0 - tiSoXuyen*tiSoXuyen*a;     {cos θ2}

         IF b < 0.0 THEN  {θ2 > 90º, không khúc xạ, phản xạ}
            tiaKhucXa := tinhTiaPhanXa( phapTuyen, diemTrung, huongTrung )

         ELSE {θ2 < 90º, có khúc xạ}
            BEGIN
               b := SQRT(b);
               tiaKhucXa.huong.x := (huongTrung.x - phapTuyen.x*tichVoHuong)*tiSoXuyen + phapTuyen.x*b;
               tiaKhucXa.huong.y := (huongTrung.y - phapTuyen.y*tichVoHuong)*tiSoXuyen + phapTuyen.y*b;
               tiaKhucXa.huong.z := (huongTrung.z - phapTuyen.z*tichVoHuong)*tiSoXuyen + phapTuyen.z*b;

               {cộng thêm một chút cho ra vật thể}
               tiaKhucXa.goc.x := diemTrung.x + 0.001*tiaKhucXa.huong.x;
               tiaKhucXa.goc.y := diemTrung.y + 0.001*tiaKhucXa.huong.y;
               tiaKhucXa.goc.z := diemTrung.z + 0.001*tiaKhucXa.huong.z;
            END;
       END;

   tinhTiaKhucXa := tiaKhucXa;
END;


{tính cao quang}
FUNCTION tinhCaoQuang( huongAnhSang, huongPhanXa:Vecto; mauAnhSang:Mau; mu:SINGLE ): Mau;

VAR
   doSang:SINGLE; {độ sáng ánh sáng}
   mauCaoQuang:Mau; {màu cao quang}

BEGIN
   {tính độ sáng cao quang từ nguồn ánh sáng}
   doSang := huongAnhSang.x*huongPhanXa.x + huongAnhSang.y*huongPhanXa.y + huongAnhSang.z*huongPhanXa.z;

   {hướng ánh sáng nên nghích chiếu với pháp tuyến}
   doSang := -doSang;

   {tích vô hướng nhỏ 0 không có cao quang}
   IF doSang < 0.0  THEN
      doSang := 0.0
   ELSE
      doSang := EXP( mu * LN( doSang ));

   {xài độ sáng chỉnh màu nguồn áng sáng}
   mauCaoQuang.d := doSang*mauAnhSang.d;
   mauCaoQuang.l := doSang*mauAnhSang.l;
   mauCaoQuang.x := doSang*mauAnhSang.x;
   mauCaoQuang.dd := 1.0;

   tinhCaoQuang := mauCaoQuang;
END;


{gồm màu}
FUNCTION gomMau( mauTanXa, mauPhanXa, mauKhucXa, mauCaoQuang:Mau; tiSoPhanXa, doDuc:SINGLE ): Mau;

VAR
   mauTo:Mau;
   nghichTiSoPhanXa:SINGLE;
   nghichDoDuc:SINGLE;

BEGIN
   {nếu độ đục = 1.0 không cần khúc xạ}
   IF doDuc = 1.0 THEN
      BEGIN
         {không phản xạ chỉ chép màu tán xạ}
         IF tiSoPhanXa = 0.0 THEN
            BEGIN
               mauTo.d := mauTanXa.d;
               mauTo.l := mauTanXa.l;
               mauTo.x := mauTanXa.x;
               mauTo.dd := 1.0;
            END

         ELSE
            BEGIN
               nghichTiSoPhanXa := 1.0 - tiSoPhanXa;
               mauTo.d := nghichTiSoPhanXa*mauTanXa.d + tiSoPhanXa*mauPhanXa.d + mauCaoQuang.d;
               mauTo.l := nghichTiSoPhanXa*mauTanXa.l + tiSoPhanXa*mauPhanXa.l + mauCaoQuang.l;
               mauTo.x := nghichTiSoPhanXa*mauTanXa.x + tiSoPhanXa*mauPhanXa.x + mauCaoQuang.x;
               mauTo.dd := 1.0;
            END;
      END
   ELSE {độ đục < 1.0}
      BEGIN
         nghichDoDuc := 1.0 - doDuc;
         {nếu không có phản xạ chỉ gồm màu tán xạ và khúc xạ}
         IF tiSoPhanXa = 0.0 THEN
            BEGIN
               mauTo.d := doDuc*mauTanXa.d + nghichDoDuc*mauKhucXa.d;
               mauTo.l := doDuc*mauTanXa.l + nghichDoDuc*mauKhucXa.l;
               mauTo.x := doDuc*mauTanXa.x + nghichDoDuc*mauKhucXa.x;
               mauTo.dd := 1.0;
            END

        ELSE {gồm hết màu tán xạ, khúc xạ, phản xạ}
           BEGIN
              nghichTiSoPhanXa := 1.0 - tiSoPhanXa;
              mauTo.d := nghichTiSoPhanXa*(doDuc*mauTanXa.d + nghichDoDuc*mauKhucXa.d) + tiSoPhanXa*mauPhanXa.d + mauCaoQuang.d;
              mauTo.l := nghichTiSoPhanXa*(doDuc*mauTanXa.l + nghichDoDuc*mauKhucXa.l) + tiSoPhanXa*mauPhanXa.l + mauCaoQuang.l;
              mauTo.x := nghichTiSoPhanXa*(doDuc*mauTanXa.x + nghichDoDuc*mauKhucXa.x) + tiSoPhanXa*mauPhanXa.x + mauCaoQuang.x;
              mauTo.dd := 1.0;
          END;
      END;

   gomMau := mauTo;
END;

{tìm vật thể nào gần nhất và tô (tính) màu}
FUNCTION toMauChoTiaDo( tiaDo:Tia; hinhCauNho, hinhCauLon:HinhCau; matPhangNen:MatPhang; soNhoi:BYTE ): Mau;

VAR
   thongTinVatTheGanNhat:ThongTinTrung;
   thongTinHinhCauNho:ThongTinTrung;
   thongTinHinhCauLon:ThongTinTrung;
   thongTinMatPhangNen:ThongTinTrung;
   mauTo:Mau;
   mauNguonAnhSang:Mau;
   huongNguonAnhSang:Vecto;
   mauTanXa:Mau;
   mauKhucXa:Mau;
   mauPhanXa:Mau;
   mauCaoQuang:Mau;
   tiaKhucXa:Tia;
   tiaPhanXa:Tia;

BEGIN
   {đặt chưa trúng gì cả}
   thongTinVatTheGanNhat.cachXa := kVO_CUC;

   {xem vật thể nào gần nhất}
   thongTinHinhCauNho := xemCatHinhCau( hinhCauNho, tiaDo );
   IF thongTinHinhCauNho.cachXa < thongTinVatTheGanNhat.cachXa THEN
      thongTinVatTheGanNhat := thongTinHinhCauNho;

   thongTinHinhCauLon := xemCatHinhCau( hinhCauLon, tiaDo );
   IF thongTinHinhCauLon.cachXa < thongTinVatTheGanNhat.cachXa THEN
      thongTinVatTheGanNhat := thongTinHinhCauLon;

   thongTinMatPhangNen := xemCatMatPhang( matPhangNen, tiaDo );
   IF thongTinMatPhangNen.cachXa < thongTinVatTheGanNhat.cachXa THEN
      thongTinVatTheGanNhat := thongTinMatPhangNen;

   {nếu tia cắt vất thể nào tính màu cho nó}
   IF thongTinVatTheGanNhat.cachXa < kVO_CUC THEN
     BEGIN
     {đặt màu nguồn ánh sáng}
      mauNguonAnhSang.d := 1.0;
      mauNguonAnhSang.l := 1.0;
      mauNguonAnhSang.x := 1.0;
      mauNguonAnhSang.dd := 1.0;

      {hướng từ -1; -1; -1 và đơn vị hóa}
      huongNguonAnhSang.x := -0.66667;
      huongNguonAnhSang.y := -0.66667;
      huongNguonAnhSang.z := -0.33333;

      {tính màu khuếch xạ}
      mauTanXa := tinhMauTanXa( mauNguonAnhSang, thongTinVatTheGanNhat.mauTo, huongNguonAnhSang, thongTinVatTheGanNhat.phapTuyen );

      {màu phản xạ}
      mauPhanXa.d := 0.0;
      mauPhanXa.l := 0.0;
      mauPhanXa.x := 0.0;
      mauPhanXa.dd := 1.0;

      {màu khúc xạ}
      mauKhucXa.d := 0.0;
      mauKhucXa.l := 0.0;
      mauKhucXa.x := 0.0;
      mauKhucXa.dd := 1.0;

      {màu cao quang}
      mauCaoQuang.d := 0.0;
      mauCaoQuang.l := 0.0;
      mauCaoQuang.x := 0.0;
      mauCaoQuang.dd := 1.0;

      {nếu số nhồi tia phản xạ ít hơn lượng tối đa}
      IF soNhoi + 1 < kNHOI_TOI_DA THEN
         BEGIN
            {nếu có phàn xạ tính màu phản xạ}
            IF thongTinVatTheGanNhat.tiSoPhanXa > 0.0 THEN
               BEGIN
                  {tính hướng phàn xạ}
                  tiaPhanXa := tinhTiaPhanXa( thongTinVatTheGanNhat.phapTuyen, thongTinVatTheGanNhat.diemTrung, thongTinVatTheGanNhat.huongTiaTrung );

                   mauPhanXa := toMauChoTiaDo( tiaPhanXa, hinhCauNho, hinhCauLon, matPhangNen, soNhoi + 1 );

                   {màu cao quang}
                   mauCaoQuang := tinhCaoQuang( huongNguonAnhSang, tiaPhanXa.huong, mauNguonAnhSang, 250.0 );
               END;
         END;

         {tính khúc xạ}
         IF thongTinVatTheGanNhat.mauTo.dd < 1.0 THEN
            BEGIN
               {tính tia khúc xạ}
               tiaKhucXa := tinhTiaKhucXa( thongTinVatTheGanNhat.phapTuyen, thongTinVatTheGanNhat.diemTrung, tiaDo.huong, thongTinVatTheGanNhat.chietSuat );
               mauKhucXa := toMauChoTiaDo( tiaKhucXa, hinhCauNho, hinhCauLon, matPhangNen, soNhoi + 1 );
            END;

      {gồm màu}
      mauTo := gomMau( mauTanXa, mauPhanXa, mauKhucXa, mauCaoQuang, thongTinVatTheGanNhat.tiSoPhanXa, thongTinVatTheGanNhat.mauTo.dd );

      END
   ELSE {màu bầu trời}
      BEGIN
         mauTo.d := 0.0;  {màu đen}
         mauTo.l := 0.0;
         mauTo.x := 0.0;
         mauTo.dd := 1.0;
      END;

   toMauChoTiaDo := mauTo;
END;


{------------------------------------------------------------------}
PROCEDURE luuThongTinKenhFloat_EXR(VAR tep:DuLieuTepAnh; danhSachKenh:MangTenKenh; soLuongKenh:BYTE );

VAR
   chiSoKenh:BYTE;
   beDaiDuLieuKenh:BYTE;

BEGIN
   WRITE( tep, $63 );  {'c'}
   WRITE( tep, $68 );  {'h'}
   WRITE( tep, $61 );  {'a'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $6c );  {'l'}
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, 0 );

   WRITE( tep, $63 );  {'c'}
   WRITE( tep, $68 );  {'h'}
   WRITE( tep, $6c );  {'l'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $74 );  {'t'}
   WRITE( tep, 0 );
   beDaiDuLieuKenh := soLuongKenh*18 + 1;
   WRITE( tep, beDaiDuLieuKenh );  {bề dài cho 4 kênh, tên các kênh dài một chữ cái ASCII}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   {thông tin cho các kênh}
   chiSoKenh := 0;
   WHILE chiSoKenh < soLuongKenh DO
      BEGIN
         WRITE( tep, danhSachKenh[chiSoKenh] );
         WRITE( tep, 0 );

         WRITE( tep, 2 );  {kiểu dữ liệu 0x01 nghỉa là float}
         WRITE( tep, 0 );
         WRITE( tep, 0 );
         WRITE( tep, 0 );

         WRITE( tep, 0 );  {chỉ xài cho phương pháp nén B44, ở đây không xài}
         WRITE( tep, 0 );
         WRITE( tep, 0 );
         WRITE( tep, 0 );

         WRITE( tep, 1 );  {nhịp x, mẫu vật}
         WRITE( tep, 0 );
         WRITE( tep, 0 );
         WRITE( tep, 0 );

         WRITE( tep, 1 );  {nhịp x, mẫu vật}
         WRITE( tep, 0 );
         WRITE( tep, 0 );
         WRITE( tep, 0 );

         chiSoKenh := chiSoKenh + 1;
      END;

      {kết thúc danh sách kênh}
   WRITE( tep, 0 );
END;


{lưu thông tin về phương pháp nén}
PROCEDURE luuThongTinNen(VAR tep:DuLieuTepAnh; phuongPhapNen:BYTE );

BEGIN
   WRITE( tep, $63 );  {'c'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $6d );  {'m'}
   WRITE( tep, $70 );  {'p'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, 0 );

   WRITE( tep, $63 );  {'c'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $6d );  {'m'}
   WRITE( tep, $70 );  {'p'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, 0 );

   WRITE( tep, 1 );  {bề dài dữ liệu}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, phuongPhapNen );
END;


PROCEDURE luuThongTinCuaSoDuLieu(VAR tep:DuLieuTepAnh; beRong, beCao:INTEGER );

BEGIN
   beRong := beRong - 1; {số cột cuối}
   beCao := beCao - 1;   {số hàng cuối}
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $61 );  {'a'}
   WRITE( tep, $74 );  {'t'}
   WRITE( tep, $61 );  {'a'}
   WRITE( tep, $57 );  {'W'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $77 );  {'w'}
   WRITE( tep, 0 );

   WRITE( tep, $62 );  {'b'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $78 );  {'x'}
   WRITE( tep, $32 );  {'2'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, 0 );

   WRITE( tep, 16 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, 0 );  {góc x}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, 0 );  {góc y}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   // ---- cột cuối
   WRITE( tep, beRong AND $ff );
   WRITE( tep, (beRong >> 8) AND $ff );
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   // ---- hàng cuối
   WRITE( tep, beCao AND $ff );
   WRITE( tep, (beCao >> 8) AND $ff );
   WRITE( tep, 0 );
   WRITE( tep, 0 );
END;


PROCEDURE luuThongTinCuaSoChieu(VAR tep:DuLieuTepAnh; beRong, beCao:INTEGER );

BEGIN
   beRong := beRong - 1;  // số cột cuối
   beCao := beCao - 1;   // số hàng cuối
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $70 );  {'p'}
   WRITE( tep, $6c );  {'l'}
   WRITE( tep, $61 );  {'a'}
   WRITE( tep, $79 );  {'y'}
   WRITE( tep, $57 );  {'W'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $77 );  {'w'}
   WRITE( tep, 0 );

   WRITE( tep, $62 );  {'b'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $78 );  {'x'}
   WRITE( tep, $32 );  {'2'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, 0 );

   WRITE( tep, 16 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, 0 );  {góc x}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, 0 );  {góc y}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   // ---- cột cuối
   WRITE( tep, beRong AND $ff );
   WRITE( tep, (beRong >> 8) AND $ff );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   // ---- hàng cuối
   WRITE( tep, beCao AND $ff );
   WRITE( tep, (beCao >> 8) AND $ff );
   WRITE( tep, 0 );
   WRITE( tep, 0 );
END;


{lưu thông tin về thứ tự hàng}
PROCEDURE luuThongTinThuTuHang(VAR tep:DuLieuTepAnh; thuTuHang:BYTE );

BEGIN
   WRITE( tep, $6c );  {'l'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $4f );  {'O'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, 0 );

   WRITE( tep, $6c );  {'l'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $4f );  {'O'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, 0 );

   WRITE( tep, 1 );  {bề dài dữ liệu}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, thuTuHang );  {từ nhỏ tới lớn}
END;


{lưu thông tin tỉ số cạnh điểm ảnh = 1.0}
PROCEDURE luuThongTinTiSoCanhDiemAnh_1(VAR tep:DuLieuTepAnh );

BEGIN
   WRITE( tep, $70 );  {'p'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $78 );  {'x'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $6c );  {'l'}
   WRITE( tep, $41 );  {'A'}
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $70 );  {'p'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $63 );  {'c'}
   WRITE( tep, $74 );  {'t'}
   WRITE( tep, $52 );  {'R'}
   WRITE( tep, $61 );  {'a'}
   WRITE( tep, $74 );  {'t'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, 0 );

   WRITE( tep, $66 );  {'f'}
   WRITE( tep, $6c );  {'l'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $61 );  {'a'}
   WRITE( tep, $74 );  {'t'}
   WRITE( tep, 0 );

   WRITE( tep, 4 );   {bề dài dữ liệu}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, 0 );  {1.0}
   WRITE( tep, 0 );
   WRITE( tep, $80 );
   WRITE( tep, $3f );
END;


{lưu thông tin tọa độ tâm màn = (0,0; 0,0}
PROCEDURE luuThongTinTamMan_1(VAR tep:DuLieuTepAnh );

BEGIN
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $63 );  {'c'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $57 );  {'W'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $77 );  {'w'}
   WRITE( tep, $43 );  {'C'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $74 );  {'t'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, 0 );

   WRITE( tep, $76 );  {'v'}
   WRITE( tep, $32 );  {'2'}
   WRITE( tep, $66 );  {'f'}
   WRITE( tep, 0 );

   WRITE( tep, 8 );  {bề dài dữ liệu}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, 0 );  {tọa độ x = 0.0}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, 0 );  {tọa độ y = 0.0}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );
END;


{lưu thông tin bề rộng màn = 1.0}
PROCEDURE luuThongTinBeRongMan_1(VAR tep:DuLieuTepAnh );

BEGIN
   WRITE( tep, $73 );  {'s'}
   WRITE( tep, $63 );  {'c'}
   WRITE( tep, $72 );  {'r'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $65 );  {'e'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $57 );  {'W'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $6e );  {'n'}
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $77 );  {'w'}
   WRITE( tep, $57 );  {'W'}
   WRITE( tep, $69 );  {'i'}
   WRITE( tep, $64 );  {'d'}
   WRITE( tep, $74 );  {'t'}
   WRITE( tep, $68 );  {'h'}
   WRITE( tep, 0 );

   WRITE( tep, $66 );  {'f'}
   WRITE( tep, $6c );  {'l'}
   WRITE( tep, $6f );  {'o'}
   WRITE( tep, $61 );  {'a'}
   WRITE( tep, $74 );  {'t'}
   WRITE( tep, 0 );

   WRITE( tep, 4 );   {bề dài dữ liệu}
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   WRITE( tep, 0 );  {1.0}
   WRITE( tep, 0 );
   WRITE( tep, $80 );
   WRITE( tep, $3f );
END;


PROCEDURE luuBangDuLieuAnh(VAR tep:DuLieuTepAnh; beRong, beCao:INTEGER; soLuongKenh:BYTE );

VAR
   diaChiThanhPhan:LONGINT; {địa chỉ thành phần trong tệp}
   soLuongByteMotThanhPhanAnh:LONGINT; {bề dài thành phần ảnh, không nén ảnh cho nên bằng nhau}
   soHang:INTEGER; {số hàng ảnh của đầu thành phần}

BEGIN
   {tính địa chỉ cho thành phần đầu sau bảng, đơn vị BYTE}
   diaChiThanhPhan := FILEPOS( tep ) + (beCao << 3) + 1; {vị trí lưu trong tệp lưu tới bây giờ + bề dài bảng thành phần}
   {mỗi kênh là kiếu float cho nên nhân 4 và + 8 byte cho thông tin thông tin thành phần}
   soLuongByteMotThanhPhanAnh := (beRong*soLuongKenh << 2) + 8;

   {lưu các địa chỉ thàng phần tong bảng thành phần}
   soHang := 0;
   WHILE soHang < beCao DO
     BEGIN
      WRITE( tep, diaChiThanhPhan AND $ff );
      WRITE( tep, (diaChiThanhPhan >> 8) AND $ff );
      WRITE( tep, (diaChiThanhPhan >> 16) AND $ff );
      WRITE( tep, (diaChiThanhPhan >> 24) AND $ff );
      WRITE( tep, 0 );
      WRITE( tep, 0 ); {Pascal không có kiểu số nguyên dài 8 byte, chỉ đặt các byte này = 0}
      WRITE( tep, 0 );
      WRITE( tep, 0 );
      diaChiThanhPhan := diaChiThanhPhan + soLuongByteMotThanhPhanAnh;
      soHang := soHang + 1;
    END;
END;


PROCEDURE luuDuLieuKenh(VAR tep:DuLieuTepAnh; kenh:KenhAnh; diaChi:LONGINT; beRong:INTEGER );

VAR
   soCot:INTEGER;  {số cột}
   doiKieu:TapHop; {đổi kiểu}

BEGIN
   soCot := 0;
   WHILE soCot < beRong DO
      BEGIN
         {đổi SINGLE sang dãy BYTE cho lưu trong tệp}
         doiKieu.soThat := kenh[diaChi + soCot];
         WRITE( tep, doiKieu.dayByte[0] );
         WRITE( tep, doiKieu.dayByte[1] );
         WRITE( tep, doiKieu.dayByte[2] );
         WRITE( tep, doiKieu.dayByte[3] );
         soCot := soCot + 1;
      END;
END;


PROCEDURE luuAnh( anhKetXuat:Anh );

VAR
   tep:FILE OF BYTE; {tệp}
   danhSachKenh:MangTenKenh; {mảng tên kênh}
   soHang:INTEGER;  {số hàng trong ảnh}
   beDaiDuLieu:LONGINT; {bề rộng dữ liệu cho một thành phần, 3 kenh * cỡ thước (float) * be rộng}
   diaChi:LONGINT; {địa chỉ trong đệm dữ liệu ảnh}

BEGIN
   ASSIGN(tep, 'G:\ANH_KET_XUAT.EXR' );
   REWRITE(tep);

   {mã số EXR}
   WRITE( tep, $76 );
   WRITE( tep, $2f );
   WRITE( tep, $31 );
   WRITE( tep, $01 );

   {phiên bản 2 (chỉ phiên bản 2 được phát hành)}
   WRITE( tep, 2 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );
   WRITE( tep, 0 );

   {thông cho các kênh}
   danhSachKenh[0] := $42; {'B'}
   danhSachKenh[1] := $47; {'G'}
   danhSachKenh[2] := $52; {'R'}
   luuThongTinKenhFloat_EXR( tep, danhSachKenh, 3 );

   {nén}
   luuThongTinNen( tep, 0);  {0 = không nén}

   {cửa sổ dữ liệu}
   luuThongTinCuaSoDuLieu( tep, anhKetXuat.beRong, anhKetXuat.beCao );

   {cửa sổ dữ liệu}
   luuThongTinCuaSoChieu( tep, anhKetXuat.beRong, anhKetXuat.beCao );

   {thứ tự hàng}
   luuThongTinThuTuHang( tep, 0 );  {0 = từ nhỏ tới lớn}

   {tỉ số cạnh điểm ảnh}
   luuThongTinTiSoCanhDiemAnh_1( tep );  {1.0}

   {trung tâm cửa sổ màn}
   luuThongTinTamMan_1( tep );

   {bề rộng cửa sổ màn}
   luuThongTinBeRongMan_1( tep );

   {kết thúc phần đầu, chuổi đặc điểm}
   WRITE( tep, 0 );

   {bảng cho thành phần dữ liệu ảnh}
   luuBangDuLieuAnh( tep, anhKetXuat.beRong, anhKetXuat.beCao, 3 );

   {lưu dữ liệu cho thành phần ảnh}
   beDaiDuLieu := anhKetXuat.beRong*3 << 2; {giống cho mỗi thành phần, 3 kenh * cỡ thước (float) * be rộng}
   soHang := 0;
   WHILE soHang < anhKetXuat.beCao DO
      BEGIN
         {luư số hàng}
         WRITE( tep, soHang AND $ff );
         WRITE( tep, (soHang >> 8) AND $ff );
         WRITE( tep, (soHang >> 16) AND $ff );
         WRITE( tep, (soHang >> 24) AND $ff );

         {bề dài dữ liệu}
         WRITE( tep, beDaiDuLieu AND $ff );
         WRITE( tep, (beDaiDuLieu >> 8) AND $ff );
         WRITE( tep, (beDaiDuLieu >> 16) AND $ff );
         WRITE( tep, (beDaiDuLieu >> 24) AND $ff );

         {dữ liệu kênh}
         diaChi := anhKetXuat.beRong*(anhKetXuat.beCao - soHang - 1);
         luuDuLieuKenh( tep, anhKetXuat.kenhXanh, diaChi, anhKetXuat.beRong);
         luuDuLieuKenh( tep, anhKetXuat.kenhLuc, diaChi, anhKetXuat.beRong);
         luuDuLieuKenh( tep, anhKetXuat.kenhDo, diaChi, anhKetXuat.beRong);

         soHang := soHang + 1;
      END;

     // ---- đóng tệp
   CLOSE( tep );

END;
{-------------------------------------------------------}


VAR
   mangAnh:MangDiemAnh;   {mảng dữ liệu ảnh}
   gocX:SINGLE; {tọa độ x cho góc ảnh}
   gocY:SINGLE; {tọa độ y cho góc ảnh}
   buoc:SINGLE; {bước hay cỡ kích điềm ảnh}
   nuaBuoc:SINGLE; {1/2 bước cho đến tâm điểm ảnh}
   tiaDo: Tia;  {tia dò}
   soCot:INTEGER;  {số cột}
   soHang:INTEGER;  {số hàng}

   {cho vật thể}
   mauVatThe:Mau;  {màu vật thể}
   tamVatThe:Vecto; {tâm vật thể}
   thongTin:ThongTinTrung;
   mauTo:Mau; {tô màu}

   {các vật thể}
   hinhCauNho:HinhCau;  {hình cầu nhỏ}
   hinhCauLon:HinhCau;  {hình cầu lớn}
   matPhangNen:MatPhang; {mặt phẳng nền}

   {ảnh}
   anhKetXuat:Anh; {ảnh kêt xuất}
   chiSoMang:LONGINT; {chỉ số mảng}


BEGIN;
   {giữ cỡ thước ảnh}
   anhKetXuat.beRong := kBE_RONG;
   anhKetXuat.beCao := kBE_CAO;

   {tính bước giữa các điểm ảnh}
   buoc := 2.0/kBE_RONG;
   {1/2 bước đến tâm điểm ảnh}
   nuaBuoc := buoc*0.5;

   {tọa độ góc ảnh, phía trái trên}
   gocX := -0.5*kBE_RONG*buoc + nuaBuoc;
   gocY := -0.5*kBE_CAO*buoc + nuaBuoc;

   {góc tia là điểm nhìn cho máy quay phim}
   tiaDo.goc.x := 0.0;
   tiaDo.goc.y := 0.0;
   tiaDo.goc.z := 10.0;

   {tạo hình cầu nhỏ cho xem thử}
   {hình cầu nhỏ}
   tamVatThe.x := 1.5;
   tamVatThe.y := 1.0;
   tamVatThe.z := 0.0;

   mauVatThe.d := 0.0;
   mauVatThe.l := 0.0;
   mauVatThe.x := 1.0;
   mauVatThe.dd := 1.0;
   hinhCauNho := taoHinhCau( tamVatThe, 0.3, mauVatThe, 1.4, 0.3 );

   {hình cầu lớn}
   tamVatThe.x := -1.0;
   tamVatThe.y := -1.0;
   tamVatThe.z := 0.0;

   mauVatThe.d := 1.0;
   mauVatThe.l := 0.0;
   mauVatThe.x := 0.02;
   mauVatThe.dd := 1.0;
   hinhCauLon := taoHinhCau( tamVatThe, 1.0, mauVatThe, 1.4, 0.3 );

   {mặt phẳng nền}
   tamVatThe.x := 0.0;
   tamVatThe.y := 0.0;
   tamVatThe.z := -15.0;

   mauVatThe.d := 0.8;
   mauVatThe.l := 0.8;
   mauVatThe.x := 0.8;
   mauVatThe.dd := 1.0;
   matPhangNen := taoMatPhang( tamVatThe, 8.0, 8.0, mauVatThe, 0.9 );

   {Quét dòng ảnh và dò tia}
   FOR soHang := 0 TO kBE_CAO - 1 DO
      BEGIN
         FOR soCot := 0 TO kBE_RONG - 1 DO
            BEGIN
              {tính hướng cho các tia}
              tiaDo.huong.x := gocX + soCot*buoc;
              tiaDo.huong.y := gocY + soHang*buoc;
              tiaDo.huong.z := -5.0;

              {xem tia có cắt vật thể nào}
              mauTo := toMauChoTiaDo( tiaDo, hinhCauNho, hinhCauLon, matPhangNen, 0 );

              {tính chỉ số mảng}
              chiSoMang := soHang*kBE_RONG + soCot;

              {giữ màu tô trong các kênh của ảnh kết xuất}
              anhKetXuat.kenhDo[chiSoMang] := mauTo.d;
              anhKetXuat.kenhLuc[chiSoMang] := mauTo.l;
              anhKetXuat.kenhXanh[chiSoMang] := mauTo.x;

            END;
         writeln();
      END;

 {lưu ảnh}
   luuAnh( anhKetXuat );

END.

