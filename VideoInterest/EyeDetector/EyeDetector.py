import numpy as np
import cv2
from flask import Flask, render_template
from flask_socketio import SocketIO, emit, send

face_cascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
print(face_cascade)
eye_cascade = cv2.CascadeClassifier('haarcascade_eye.xml')
fullbright = Fals

app = Flask("serve")
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

@socketio.on('message')
def handle_message(message):
	global fullbright
	print('received message: ' + message)
	cap = cv2.VideoCapture(0)
	ret, img = cap.read()
	gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
	faces = face_cascade.detectMultiScale(gray, 1.3, 5)
	numEyes = 0
	for (x,y,w,h) in faces:
		cv2.rectangle(img,(x,y),(x+w,y+h),(255,0,0),2)
		roi_gray = gray[y:y+h, x:x+w]
		roi_color = img[y:y+h, x:x+w]
		eyes = eye_cascade.detectMultiScale(roi_gray)
		for (ex,ey,ew,eh) in eyes:
			cv2.rectangle(roi_color,(ex,ey),(ex+ew,ey+eh),(0,255,0),2)
			numEyes += 1
	if(numEyes < 1):
		f = open('/sys/class/backlight/intel_backlight/brightness', 'w')
		f.truncate()
		f.write("900")
		fullbright = True
		f.close()
		send("slow")
	elif fullbright:
		f = open('/sys/class/backlight/intel_backlight/brightness', 'w')
		f.truncate()
		f.write("400")
		fullbright = False
		f.close()
		send("fast")
	cv2.imshow("img", img)
	cv2.waitKey(500)
	cap.release()
	cv2.destroyAllWindows()

void SaveROICPP(const CStringA& inputFile, const CStringA& outputFile)
{      
    if (ATLPath::FileExists(inputFile))
    {
        cv::Mat imgInput = cv::imread(inputFile.GetString());

        if (imgInput.data != NULL)
        {
            cv::Mat imgRoi = imgInput(cv::Rect(ex,ey),(ex+ew,ey+eh));

            imgInput.copyTo(imgRoi);                 

            cv::imwrite(outputFile.GetString(), imgRoi);
        }
    }
}
	
socketio.run(app)
