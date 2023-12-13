package com.example.read_logs;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_PERMISSION_CODE = 123;
    private ListView listView;
    private ArrayAdapter<String> arrayAdapter;
    private ArrayList<String> lastLines;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        listView = findViewById(R.id.listView);
        lastLines = new ArrayList<>();
        arrayAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, lastLines);
        listView.setAdapter(arrayAdapter);

        requestPermissions();

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    while (true) {
                        Process process = Runtime.getRuntime().exec("logcat -d -v time *:V");
                        BufferedReader bufferedReader = new BufferedReader(
                                new InputStreamReader(process.getInputStream()));

                        String line;
                        while ((line = bufferedReader.readLine()) != null) {
                            final String finalLine = line;
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    handleLogLine(finalLine);
                                }
                            });
                        }

                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    private void requestPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_LOGS) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_LOGS}, REQUEST_PERMISSION_CODE);
            }
        }
    }


    private void handleLogLine(String logLine) {
        if (logLine.contains("android.intent.extra.STREAM expected Parcelable") && !lastLines.contains(logLine)) {
            lastLines.add(logLine);
            arrayAdapter.notifyDataSetChanged();

            // Extrair PID e mensagem relevante
            String pid = extractPID(logLine);
            String message = extractErrorMessage(logLine);

            // Exibir o popup
            showPopup(pid, message);
        }
    }

    private String extractPID(String logLine) {
        // Use uma expressão regular para extrair o PID da linha de log
        Pattern pattern = Pattern.compile("\\b\\((\\d+)\\):", Pattern.CASE_INSENSITIVE);
        Matcher matcher = pattern.matcher(logLine);

        if (matcher.find()) {
            return matcher.group(1);
        } else {
            return "PID não encontrado";
        }
    }

    private String extractErrorMessage(String logLine) {
        if (logLine.contains("tentou enviar um objeto inválido para o STREAM")) {
            return "O processo tentou enviar um objeto inválido para o STREAM";
        } else {
            return "Mensagem de erro não encontrada";
        }
    }

    private void showPopup(String pid, String message) {
        LayoutInflater inflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View popupView = inflater.inflate(R.layout.popup_layout, null);

        TextView popupText = popupView.findViewById(R.id.popupText);
        popupText.setText("PID: " + pid + "\n" + message);

        PopupWindow popupWindow = new PopupWindow(popupView, 600, 400, true);
        popupWindow.showAtLocation(listView, Gravity.CENTER, 0, 0);

        // Fechar o popup após alguns segundos
        new Handler(Looper.getMainLooper()).postDelayed(new Runnable() {
            @Override
            public void run() {
                popupWindow.dismiss();
            }
        }, 3000); // 3000 milissegundos (3 segundos)
    }
}
