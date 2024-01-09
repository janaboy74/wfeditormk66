# Watchface editor for MK66 and P32D Smartwatches

![image](https://github.com/janaboy74/wfeditormk66/assets/54952408/f9b74273-6cd8-4427-b086-459cfb1435c8)

This project is a Watchface editor for MK66.
- The project uses gtkmm and now it comes with binary for Linux and Windows.
- This software comes with no warranty. Use it at own risk.
- I am not responsible of any demage on the device.
- The project is in beta state.
- It can be compiled using cmake or CodeBlocks.

## Usage
Menu:
| Item | Description |
| :---: | :---: |
| Load bin | Load the watch face bin file |
| Sace bin | Save the watch face bin file |
| Quit | Quit application |
| About | About the program |

Toolbar #1:
| Item | Description |
| :---: | :---: |
| Item Types | Watchface item types |
| Load | Load the item png file |
| Sace | Save the item png file |
| New Item Types | New watchface item types |
| Add | Add new watchface item type |
| Del | Delete old watchface item type |
| X pos | X position of the item |
| Y Pos | Y position of the item |
| Shift | Item list vertical shift |

Toolbar #2:
| Item | Description |
| :---: | :---: |
| Delta height | Delta height for item images |
| Add height | Add the delta height for item images |
| Default value | Default value for the item preview |

Keys:
| Key | Description |
| :---: | :---: |
| Esc | Clear item parameters |
| F1 | Preview mode |
| F2 | Details mode |
| q | Previous file in the Watchfaces subfolder. |
| w | Next file in the Watchfaces subfolder. |
| c | Create preview image. |
| d | Switch debug mode( the preview contains IDs instead of test values ). |
| numpad - | spin button sub ( shift -> accel ) |
| numpad + | spin button add ( shift -> accel ) |
| page down | fast spin button sub |
| page up | fast spin button add |
| cursors | moving the selected item ( shift -> accel ) |
| shift + mouse | drag the selected item |

---
### Upload the watchface
#### Prerequisites for the android devices
- Glory Fit
- A file manager
#### Steps
- Download watchface(s) with Glory Fit.
- Use the file manager to download the watchfaces /Android/data/com.yc.gloryfit/files in the Main Storage to your pc ( you need to add permissions for the file manager to the gloryfit folder ).
- Edit the watchface with the editor.
- Replace an existing watchface with the edited one.
- Sync the watchface with the Glory Fit application.
- Done.

I have not found other way to upload custom watchface to the MK66.<br />
If someone has a better solution please share it with me and I will publish it.

---
### Brief compilation instructions:
- The compilation requires cmake and gtkmm3.
- For windows I have used MSys: https://www.msys2.org
- You need to install for both windows and for linux development packages as well.
---
János
---
It took 3 days until the editor was fully functional + an addition day until I found the right checksum routine waht is stored in the file.<br>
I invested around an additional week for refinements and rework until its final usable state.<br>
It took another day to implement the decompression of compressed watchfaces.
