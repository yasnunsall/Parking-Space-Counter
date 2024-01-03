import cv2

width = 100
height = 42
file_path = "data/CarParkPos.txt"

try:
    with open(file_path, "r") as f:
        pos_list = [tuple(map(int, line.split())) for line in f]
except FileNotFoundError:
    pos_list = []

def write_positions_to_file(positions):
    with open(file_path, "w") as f:
        for pos in positions:
            f.write(f"{pos[0]} {pos[1]}\n")

def mouse_click(event, x, y, flags, params):
    global pos_list

    if event == cv2.EVENT_LBUTTONDOWN:
        pos_list.append((x, y))

    if event == cv2.EVENT_RBUTTONDOWN:
        pos_list = [pos for pos in pos_list if not (pos[0] < x < pos[0] + width and pos[1] < y < pos[1] + height)]

    write_positions_to_file(pos_list)

while True:
    img = cv2.imread("firstFrame.png")
    img = cv2.resize(img, (img.shape[1], img.shape[0]))

    for pos in pos_list:
        cv2.rectangle(img, pos, (pos[0] + width,  pos[1] + height), (255, 0, 0), 2)

    cv2.imshow("img", img)
    cv2.setMouseCallback("img", mouse_click)
    key = cv2.waitKey(1) & 0xFF

    if key == ord("q"):
        cv2.imwrite("CarParkPos.png", img)
        break

cv2.destroyAllWindows()
