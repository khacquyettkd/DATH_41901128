### flask
from flask import Flask, session, redirect, url_for, request,render_template,flash,Response,jsonify
from flask_socketio import SocketIO, emit
from flask_cors import CORS
import time
import threading
from queue import Queue
from threading import Lock,Thread
#### YOLOv5
import cv2, torch
import pathlib
temp = pathlib.PosixPath
pathlib.PosixPath = pathlib.WindowsPath
#### sound
import sounddevice as sd
import numpy as np
from scipy.io.wavfile import write
import io, os, glob
import wave
#### api
from google.cloud import speech_v1 as speech
from google.cloud import texttospeech
from openai import OpenAI
### UART
import serial
import serial.tools.list_ports
import re
#Speech to text
os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = "key.json"
client = speech.SpeechClient()
#text to speech
os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = "key_text_to_speech.json"
client_texttospeech = texttospeech.TextToSpeechClient()
#client openAI
client_openAI = OpenAI(api_key="your API here",)
###----------------------------------------------FLASK----------------------------------------------------
app = Flask(__name__)
socketio = SocketIO(app)
CORS(app)
app.secret_key = 'your secret key here'
USER_CONST='Quyet'
PASS_CONST='41901128'
#### --------sound const------------
duration=10
samplerate=44100 #wav
isTextToSpeechInProgress=False
# samplerate=16000 #mp3
#### --------yolo------------
path='D:\\A.1.Kien thuc+slide cac mon\\4.senior\\ĐATN\\Web_demo\\yolov5'
path_to_model = 'D:\\A.1.Kien thuc+slide cac mon\\4.senior\\ĐATN\\Web_demo\\yolov5\\weight\\best.pt'
device = '0'  # Chọn device GPU, nếu có
recording=None
cap=None
model = torch.hub.load(path, 'custom', path_to_model, device=device, source='local')
print('Model is succesfully loaded')
#### --------UART-----------
ser = None

def read_data_thread():#read UART
    global ser
    while True:
        try:
            if ser and ser.is_open:
                data = ser.readline().decode('utf-8')
                if(data):
                    socketio.emit('command', data)
                    print("read:"+data)
                    print("read raw:", repr(data))
                    time.sleep(1)  # Thêm một khoảng thời gian ngủ để tránh lặp quá nhanh
        except Exception as e:
            print(f"Error reading data: {str(e)}")
def drawText(img,text,font_scale = 1, posX = 10, posY = 10, font = cv2.FONT_HERSHEY_DUPLEX, bgColor = (255, 255, 255), textColor = (0, 0, 0), thick_ness = 1, padding = 2):
	(dai,rong)=cv2.getTextSize(text,font,font_scale,thick_ness)[0]
	cv2.rectangle(img,(posX-padding,posY+padding),(posX+dai+padding,posY-rong-padding),bgColor,cv2.FILLED)
	cv2.putText(img,text,(posX,posY),font,font_scale,textColor,thick_ness)
def generate_frames():
    global cap
    global model
    box_thickness=1
    cap=cv2.VideoCapture(0)
    success,frame=cap.read()
    x1, x2, y1, y2 = 0, 0, 0, 0
    breakloops= False
    while True:
            success, frame = cap.read()
            if not success:
                break
            else:
                detection=model(frame)
                detection.xyxy[0]  #im1 predictions (tensor)
                result=detection.pandas().xyxy[0].to_dict(orient="records") # im1 predictions (pandas)
                for i in range(0,len(result)):
                    confidence=result[i]['confidence']*100
                    name=result[i]['name']
                    # clas=result[i]['class']
                    x1=int(result[i]['xmin'])
                    x2=int(result[i]['xmax'])
                    y1=int(result[i]['ymin'])
                    y2=int(result[i]['ymax'])
                    print(name)
                    print(confidence)
                    cv2.rectangle(frame,(x1,y1),(x2,y2),(0,255,0),box_thickness)
                    drawText(frame,name+str(round(confidence,1)),1,x1,y1,cv2.FONT_HERSHEY_PLAIN,(0,255,0),(0,0,0),2,4)
                    if(confidence>70):
                        socketio.emit('detectCompletely', {'name': name,'confidence': confidence})
                        breakloops=True
                        break    
                
                if(breakloops==True):
                    break
                ret, buffer = cv2.imencode('.jpg', frame)
                frame = buffer.tobytes()
                yield (b'--frame\r\n'
                       b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
def kiem_tra_va_xoa_tep_cu_nhat(thu_muc, so_luong_toi_da=10, so_luong_xoa=6):
    duong_dan = os.path.join(os.getcwd(), thu_muc)
    danh_sach_tep = glob.glob(os.path.join(duong_dan, '*'))
    so_luong_tep = len(danh_sach_tep)
    if so_luong_tep >= so_luong_toi_da:
        danh_sach_tep.sort(key=os.path.getmtime)
        for i in range(so_luong_xoa):
            tep_cu_nhat = danh_sach_tep[i]
            os.remove(tep_cu_nhat)
        print(f'Đã xóa "{so_luong_xoa}" tệp mp3 cũ nhất tại {thu_muc}.')
def check_message(chuoi):
    pattern = r"\(1-[\d]{24}\)|\(2-[\d]{24}\)"
    if re.match(pattern, chuoi):
        print("Đúng")
        return True
    else:
        print("Thông điệp không đúng cấu trúc.")
        return False             
@app.route('/',methods=['GET','POST']) #login
def home():
    return login()
@app.route('/main',methods=['GET','POST']) #Trang chủ
# @login_required
def main():

    return render_template('main.html')

@app.route('/Login',methods=['GET','POST']) #login
def login_1():
    return login()

@app.route('/login',methods=['GET','POST']) #login
def login():
    if request.method == 'POST':
        user_get = request.form['tendangnhap'] 
        pass_get = request.form['matkhau'] 
        if(user_get==USER_CONST and pass_get==PASS_CONST):
            flash('Đăng nhập thành công')
            return render_template('main.html')
        else:
            flash('Đăng nhập thất bại. Tài khoản hoặc mật khẩu không chính xác!')
            return render_template('login.html')
    else:
        return render_template('login.html')
@app.route('/templates/hardware.html') #phần cứng
def hardware():
    return render_template('hardware.html')
@app.route('/templates/software.html') #phần mềm
def software():
    return render_template('software.html')
@app.route('/templates/tonghop.html') #Muc dich + khat vong + ket qua
def tonghop():
    return render_template('tonghop.html')
@app.route('/templates/info.html') #info
def info():
    return render_template('info.html')
@app.route('/templates/this_product.html') #this product
def this_product():
    return render_template('this_product.html')

@app.route('/templates/learn.html') #Bai hoc
def learn():
    return render_template('learn.html') 
@app.route('/templates/detect.html') #Nhan dien
def detect():
    return render_template('detect.html')
@app.route('/templates/help.html') # Tro giup
def help():
    return render_template('help.html')
@app.route('/templates/about.html') #Ve chung toi
def about():
    return render_template('about.html')

@app.route('/video_feed', methods=['POST','GET']) #Bat webcam
def video_feed():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')
@app.route('/offcam', methods=['POST']) #Tat webcam
def offcam():
    global cap
    if cap is not None:
        cap.release()
        return {'status': 'success'}
    else:
        print('cap is none')
        return {'status': 'error', 'message': 'Camera is not initialized.'}
@socketio.on('start_recording') #socket nhận lệnh ghi âm
def start_recording():
    global samplerate
    global recording
    global duration
    print("Recording...")
    recording = sd.rec(int(samplerate * duration), samplerate=samplerate, channels=1, dtype='int16')
    # return 'Recorded'
@socketio.on('stop_recording') #socket nhận lệnh tắt ghi âm
def stop_recording():
    global samplerate
    global recording
    global content_text_to_speech
    sd.stop()
    audio_file_path = 'recorded_audio.wav'
    # Lưu vào file WAV
    wave_file = wave.open(audio_file_path, 'wb')
    wave_file.setnchannels(1)
    wave_file.setsampwidth(2)
    wave_file.setframerate(samplerate)
    wave_file.writeframes(recording.tobytes())
    wave_file.close()
    print(f"Recording saved to {audio_file_path}")
    with io.open(audio_file_path, "rb") as audio_file:
        content = audio_file.read()
    audio = speech.RecognitionAudio(content=content)
    config = speech.RecognitionConfig(
        # encoding='MP3',
        # sample_rate_hertz=16000,
        encoding='LINEAR16',
        sample_rate_hertz=44100,
        language_code="vi-VN", 
    )
    # Detects speech in the audio file
    # client = speech.SpeechClient()
    response = client.recognize(config=config, audio=audio)
    for result in response.results:
        content=format(result.alternatives[0].transcript)
        print("content:", content)
        emit('audio_saved', {'message': content})
@socketio.on('text_to_speech') #socket nhận lệnh tạo speech
def chat_to_openai(message):
    #generate text form chatGPT
    global isTextToSpeechInProgress
    if(not isTextToSpeechInProgress):
        isTextToSpeechInProgress=True
        message= "Nói ngắn gọn về: "+message #--------------important---------------------------------------
        print("message:", message)
        chat_completion = client_openAI.chat.completions.create(
            messages=[
                {
                    "role": "user",
                    "content": message,
                }
            ],
            model="gpt-3.5-turbo",
        )
        text_output=chat_completion.choices[0].message.content
        print(chat_completion.choices[0].message.content)
        #generate voice and write to mp3 file
        synthesis_input = texttospeech.SynthesisInput(text=text_output)
        voice = texttospeech.VoiceSelectionParams(
            language_code="vi-VN",
            name="vi-VN-Standard-C",
            ssml_gender=texttospeech.SsmlVoiceGender.FEMALE,
        )
        audio_config = texttospeech.AudioConfig(
            audio_encoding=texttospeech.AudioEncoding.MP3,
            speaking_rate=1.25,
            pitch=0
        )
        # Tạo yêu cầu synthesis
        response = client_texttospeech.synthesize_speech(input=synthesis_input, voice=voice, audio_config=audio_config)

        # Ghi âm thanh vào tệp
        timestamp=str(int(time.time()))
        output_file="static/Audio/speech/Output"+timestamp+".mp3"
        with open(output_file, "wb") as out_file:
            out_file.write(response.audio_content)
        print(f'Nội dung âm thanh đã được ghi vào tệp "{output_file}".')
        emit('speech_created',{'timestamp': timestamp})
        isTextToSpeechInProgress=False
        kiem_tra_va_xoa_tep_cu_nhat("static/Audio/speech/", so_luong_toi_da=10, so_luong_xoa=6)
    else:
        print("OpenAI đang bận")
@app.route('/templates/jarvis.html') #jarvis
def jarvis():
    return render_template('jarvis.html')

@app.route('/templates/test_com.html')
def index():
    return render_template('test_com.html')
# @app.route('/send_data', methods=['POST'])
# def send_data():
#     global ser
#     try:
#         data = request.json['data']
#         data='('+data+')'
#         if ser and check_message(data):
#             data_bytes = data.encode('utf-8')
#             for byte in data_bytes:
#                 ser.write(bytes([byte]))
#             # ser.write(b'\n')
#             print("sent: "+data)
#             return jsonify({'status': 'success', 'message': f'Data sent: {data}'})
#         else:
#             return jsonify({'status': 'error', 'message': 'COM port not open'})
#     except Exception as e:
#         return jsonify({'status': 'error', 'message': str(e)})

uart_lock = Lock()
@app.route('/send_data', methods=['POST'])
def send_data():
    global ser
    try:
        data = request.json['data']
        data = '(' + data + ')'
        
        with uart_lock:
            if ser and check_message(data):
                data_bytes = data.encode('utf-8')
                for byte in data_bytes:
                    ser.write(bytes([byte]))
                print("sent: " + data)
                return jsonify({'status': 'success', 'message': f'Data sent: {data}'})
            else:
                return jsonify({'status': 'error', 'message': 'COM port not open'})
    except Exception as e:
        return jsonify({'status': 'error', 'message': str(e)})

    
@app.route('/connect_com', methods=['POST'])
def connect_com():
    global ser
    try:
        com_port = request.json['com_port']
        ser = serial.Serial(com_port, baudrate=9600, timeout=1, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)
        threading.Thread(target=read_data_thread, daemon=True).start()
        return jsonify({'status': 'success', 'message': f'Connected to COM port {com_port}'})
    except Exception as e:
        return jsonify({'status': 'error', 'message': str(e)})
@app.route('/list_com_ports')
def list_com_ports():
    try:
        ports = [port.device for port in serial.tools.list_ports.comports()]
        return jsonify({'status': 'success', 'ports': ports})
    except Exception as e:
        return jsonify({'status': 'error', 'message': str(e)})
@socketio.on('checkCOMConnect') 
def checkCOMConnect():
    global ser
    try:
        if ser.isOpen():
            conStatus="Connected"
        else:
            conStatus="Disconnected"
    except AttributeError:
        conStatus="Disconnected"
    
    socketio.emit('ConStatusResponse', conStatus)
@socketio.on('closePort') 
def closePort():
    global ser
    ser.close()
    print("The COM port is closed")
    
if __name__ == '__main__':
    socketio.run(app, debug=True)
