package com.example.retainfacencnn;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import java.io.FileNotFoundException;


public class MainActivity extends AppCompatActivity {


    private static final int SELECT_IMAGE = 1;

    private ImageView imageView;
    private Bitmap bitmap = null;
    private Bitmap yourSelectedImage = null;

    // retinaface
    private RetinaFace retinaface = new RetinaFace();

    // scrfd
    private SCRFD scrfd = new SCRFD();

    private SCRFDDNN scrfddnn = new SCRFDDNN();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

//        boolean ret_init = retinaface.Init(getAssets());
        boolean ret_init = scrfd.Init(getAssets());
//        boolean ret_init = scrfddnn.Init(getAssets());
        if (!ret_init) {
            Log.e("MainActivity", "model Init failed");
        }


        imageView = (ImageView) findViewById(R.id.iv_image);

        Button btn_image = findViewById(R.id.btn_select_image);
        btn_image.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent i = new Intent(Intent.ACTION_PICK);
                i.setType("image/*");
                startActivityForResult(i, SELECT_IMAGE);
            }
        });

        Button btn_cpu = findViewById(R.id.btn_cpu);
        btn_cpu.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (yourSelectedImage == null) return;

                Log.v("go","**********************************");

//                float[] face_info = retinaface.Detect(yourSelectedImage);
                float[] face_info = scrfd.Detect(yourSelectedImage);
//                float[] face_info = scrfddnn.Detect(yourSelectedImage);

                Log.v("go","**********************************");

                if (face_info == null) {
                    Toast.makeText(getApplicationContext(), "未检测到人脸", Toast.LENGTH_SHORT).show();
                    return;
                }
                Log.v("MainActivity", String.valueOf(face_info.length));
                showObjects(face_info);

            }
        });
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode == RESULT_OK && null != data) {
            Uri selectedImage = data.getData();

            try {
                if (requestCode == SELECT_IMAGE) {
                    bitmap = decodeUri(selectedImage);

                    yourSelectedImage = bitmap.copy(Bitmap.Config.ARGB_8888, true);

                    imageView.setImageBitmap(bitmap);
                }
            } catch (FileNotFoundException e) {
                Log.e("MainActivity", "FileNotFoundException");
                return;
            }
        }
    }

    private Bitmap decodeUri(Uri selectedImage) throws FileNotFoundException {
        // Decode image size
        BitmapFactory.Options o = new BitmapFactory.Options();
        o.inJustDecodeBounds = true;
        BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImage), null, o);

        // The new size we want to scale to
        final int REQUIRED_SIZE = 400;

        // Find the correct scale value. It should be the power of 2.
        int width_tmp = o.outWidth, height_tmp = o.outHeight;
        int scale = 1;
        while (true) {
            if (width_tmp / 2 < REQUIRED_SIZE
                    || height_tmp / 2 < REQUIRED_SIZE) {
                break;
            }
            width_tmp /= 2;
            height_tmp /= 2;
            scale *= 2;
        }

        // Decode with inSampleSize
        BitmapFactory.Options o2 = new BitmapFactory.Options();
        o2.inSampleSize = scale;
        return BitmapFactory.decodeStream(getContentResolver().openInputStream(selectedImage), null, o2);
    }

    private void showObjects(float[] objects) {
        // draw objects on bitmap
        Bitmap rgba = bitmap.copy(Bitmap.Config.ARGB_8888, true);

        Canvas canvas = new Canvas(rgba);

        Paint paint = new Paint();
        paint.setColor(Color.RED);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(4);

        Paint paint2 = new Paint();
        paint2.setColor(Color.GREEN);
        paint2.setStyle(Paint.Style.STROKE);
        paint2.setStrokeWidth(4);

        Paint paint3 = new Paint();
        paint3.setColor(Color.GREEN);
        paint3.setStyle(Paint.Style.STROKE);
        paint3.setStrokeWidth(2);


        for(int i=0;i<objects.length/15;i++){
            canvas.drawRect(objects[i*15], objects[i*15+1], objects[i*15+2], objects[i*15+3], paint);
            canvas.drawPoints(objects, i*15+4, 10, paint2);
            canvas.drawText(String.valueOf(objects[i*15+14]), objects[i*15], objects[i*15+1], paint3);
        }

//        for(int i=0;i<objects.length/5;i++){
//            canvas.drawPoints(objects, i*5, 4, paint2);
//            canvas.drawText(String.valueOf(objects[i*5+4]), objects[i*15], objects[i*15+1], paint3);
//        }

        imageView.setImageBitmap(rgba);
    }

}
