#!/bin/bash -x
# BFU installation wizard by Sleep_Walker ;)
# fixes and hints are welcome sleep_walker@hackndev.com
# questions in hackndev.com forum or on IRC #hackndev @ irc.freenode.net
# GPL v2

# this is list of supported devices
# first string on line is acronym of device used in release list
DIALOG_TIMEOUT=2

KED_T3_RELEASE="k106"
KED_27x_RELEASE="k27x.07"
RASTER_RELEASE="2008-11-13"

# auryn files which I don't use
#	- zImage & modules - both taken from other source
#	- squashfs - I don't handle this filetype yet, maybe added in future
#	- ext2.bz2 - I could handle this and have this loopback on card when I have kernel with initramfs/initrd which will mount it
#		   - so add release first
FILTER="(squashfs$|ext2.bz2$|/zImage|/modules)"

DEVICE_LIST="TT Tungsten|T
T3 Tungsten|T3
T5 Tungsten|T5
TX Palm TX
T650 Treo650
T680 Treo680
C Centro
LD LifeDrive
GEN Generic"

# this is list of supported releases
#	first string means what device it is for
#	second string is acronym of this release
#	rest of line is description
RELEASE_LIST="TT mx-TT Marex's release for Tungsten|T (outdated and MMC only)
T3 ked-T3 kEdAR's release $KED_T3_RELEASE for Tungsten|T3
T3 ked-sw-T3 Sleep_Walker's kernel with kEdAR's release for Tungsten|T3
T5 m-s-T5 miska & snua12's release for Tungsten|T5
TX mis-TX miska's release for PalmTX
Z71 mx-Z71 Marex's OPIE release for Zire71
Z72 z72ka-Z72 z72ka's OPIE release for Zire72
T650 rast-T650 raster's Illume image $RASTER_RELEASE for Treo650
T650 deb-T650 Alex's Debian Lenny (mainly for development) release for Treo650
T680 ked-sw-T680 Sleep_Walker's kernel with kEdAR's release for Treo680
T680 sw-mis-T680 Sleep_Walker's kernel with miska's rootfs for Treo680
LD mx-tp2-LD Marex's Technology Preview 2
GEN ked-pxa kEdAR's generic PXA27x release $KED_27x_RELEASE"

NEEDS_PARTITION="mx-TT, sw-mis-T680, rast-t650, deb-t650"
NOT_COCOBOOT="mx-TT, mx-Z71"

# if it is not special case, I want cocoboot!
OMMIT_COCOBOOT="false"

#####################################################################################
############################# General functions #####################################
#####################################################################################

is_true() {
  case "$1" in
    "on"|"yes"|"true"|"1")
      return 0;;
    *)
      return 1;;
  esac
}

only_root_pass() {
  if [ "$UID" != 0 ]; then
    echo "This script needs to be run by root."
    echo "You need root rights for card partitioning, filesystem creation, loopback mounting and writing to EXT partition."
    echo "su or login as root and try again, please."
    exit 1
  fi
}

online_update() {
  TITLE="Online update of this script"
  if is_true `$get_bool "Now I'll try to catch up-to-date configuration from Internet, this will be small ammount of data..."`; then
      # online update of functions
      UPDATE="`wget http://sleepwalker.hackndev.com/install_script.sh -o /dev/null -O -`"
      source /dev/stdin <<< "$UPDATE"
  fi
}

add_temp_file() {
  if [ -z "$TEMP_COUNT" ]; then
    TEMP_COUNT=0
  fi
  TEMP_FILES[TEMP_COUNT]="$1"
  TEMP_COUNT=$((TEMP_COUNT + 1))
}

ask_and_add_temp_file() {
  if is_true `$get_bool "Should I remove downloaded file $1"`; then
    add_temp_file "$1"
  fi
}

#####################################################################################
########################## Console dialog functions #################################
#####################################################################################

cons_error() {
  dialog --title "$TITLE" --msgbox "Error occured: \n$@" 10 40
}

cons_fatal_error() {
  cons_error "$@"
  clean_work
  exit 1
}


cons_info() {
  dialog --title "$TITLE" --infobox "$@" 10 40 
}

cons_wait_info() {
  dialog --title "$TITLE" --msgbox "$@" 10 40 
}


cons_get_bool() {
  dialog --title "$TITLE" --yesno "$@" 10 40 2>&1 > /dev/tty
  [ $? = 0 ] && echo "yes" || echo "no"
}

cons_get_string() {
  dialog --title "$TITLE" --inputbox "$@" 10 60 2>&1 > /dev/tty
}

cons_get_choice() {
  # read choices into array
  unset CHOICES
  I=0
  while read line; do
    CHOICES[$((I++))]="${line%% *}"
    CHOICES[$((I++))]="${line#* }"
  done <<< "$2"
  dialog --title "$TITLE" --menu "$1" 20 60 "$((I / 2))" "${CHOICES[@]}" 2>&1 > /dev/tty
}

cons_download() {
# $1	what to download
# $2	where into
# NOTE: here must be limitation to handle it easily
#       $2 can be dir, if it already exists or if it ends with '/'

  if [ "$2" != "${2%/}" ]; then
    # it has '/' at the end or it is existing directory
    TARGET="$2/${1##*/}"
    [ -d "$2" ] || mkdir -p "$2" || error "Cannot create $2"
  elif [ -d "$2" ]; then
    # it's existing directory
    TARGET="$2/${1##*/}"
  else
    # $2 is file
    TARGET="$2"
  fi

  # download with output to dialog progress bar
  wget "$1" -O "$TARGET" -o /dev/stdout | sed -n 's/.* \([[:digit:]]\+\)% .*/\1/p' | dialog --gauge "Downloading:\n$1\ninto\n$2" 10 40 0
}


#####################################################################################
############################## Kdialog functions ####################################
#####################################################################################

kde_error() {
  kdialog --title "$TITLE" --error "Error occured: <br>${@//\\n/<br>}"
  exit 1
}

kde_fatal_error() {
  kde_error "$@"
  clean_work
  exit 1
}

kde_info() {
  ( kdialog --title "$TITLE" --passivepopup "${@//\\n/<br>}" &
    sleep $DIALOG_TIMEOUT
    kill %1 ) &
}

kde_wait_info() {
  kdialog --title "$TITLE" --msgbox "${@//\\n/<br>}"
}


kde_get_bool() {
  kdialog --title "$TITLE" --yesno "${@//\\n/<br>}"
  [ $? = 0 ] && echo "yes" || echo "no"
}

kde_get_string() {
  kdialog --title "$TITLE" --inputbox "${@//\\n/<br>}"
}

kde_get_choice() {
  # read choices into array
  unset CHOICES
  I=0
  while read line; do
    CHOICES[$((I++))]="${line%% *}"
    CHOICES[$((I++))]="${line#* }"
  done <<< "$2"
  kdialog --title "$TITLE" --menu "${1//\\n/<br>}" "${CHOICES[@]}"
}

kde_download() {
# $1	what to download
# $2	where into
# NOTE: here must be limitation to handle it easily
#       $2 can be dir, if it already exists or if it ends with '/'


  if [ "$2" != "${2%/}" ]; then
    # it has '/' at the end or it is existing directory
    TARGET="$2/${1##*/}"
    [ -d "$2" ] || mkdir -p "$2" || error "Cannot create $2"
  elif [ -d "$2" ]; then
    # it's existing directory
    TARGET="$2/${1##*/}"
  else
    # $2 is file
    TARGET="$2"
  fi


  J=0
  # download with output to dialog progress bar
  KDIALOG="`kdialog --title "$TITLE" --progressbar "Downloading:<br>$1<br>into<br>$2"`"
  if grep "^org.kde.kdialog" <<< "$KDIALOG" > /dev/null; then
    # KDE4 kdialog and dbus control
    autoclose="qdbus $KDIALOG org.freedesktop.DBus.Properties.Set org.kde.kdialog.ProgressDialog autoClose true"
    showcancel="qdbus $KDIALOG org.kde.kdialog.ProgressDialog.showCancelButton true"
    setprogress="qdbus $KDIALOG org.freedesktop.DBus.Properties.Set org.kde.kdialog.ProgressDialog value"
    manualclose="qdbus $KDIALOG org.kde.kdialog.ProgressDialog.close"
    wascancelled="qdbus $KDIALOG org.kde.kdialog.ProgressDialog.wasCancelled"
  else
    # KDE3 kdialog and dcop control
    # cut beginning
    KDIALOG="${KDIALOG//DCOPRef(/}"
    # cut end
    KDIALOG="${KDIALOG//,ProgressDialog)/}"
    autoclose="dcop $KDIALOG ProgressDialog setAutoClose true"
    setprogress="dcop $KDIALOG ProgressDialog setProgress"
    showcancel="dcop $KDIALOG ProgressDialog showCancelButton true"
    manualclose="dcop $KDIALOG ProgressDialog close"
    wascancelled="dcop $KDIALOG ProgressDialog wasCancelled"
  fi
  $autoclose
  $showcancel
    ( wget "$1" -O "$TARGET" -o /dev/stdout | while read I; do
      I="${I//*.......... .......... .......... .......... .......... /}"
      I="${I%%%*}"
      # report changes only
      if [ "$I" ] && [ "$J" != "$I" ]; then
	$setprogress $I
	J="$I"
      fi      
    done ; $manualclose ) &
  pid="$!"
  while [ -d "/proc/$pid/" ]; do
    if [ "`$wascancelled`" = true ]; then
      kill $pid
      kill `ps aux | grep "wget $1 -O $TARGET -o /dev/stdout" | grep -v grep | tr -s ' ' ' ' | cut -f2 -d\  `
      $manualclose
    fi
    sleep 1
  done
  wait
}

#####################################################################################
############################## zenity functions #####################################
#####################################################################################

gtk_error() {
  zenity --title "$TITLE" --error --text="Error occured: \n$@"
}

gtk_fatal_error() {
  gtk_error "$@"
  clean_work
  exit 1
}

gtk_info() {
  zenity --timeout=$DIALOG_TIMEOUT --title "$TITLE" --info --text="$@" &
}

gtk_wait_info() {
  zenity --title "$TITLE" --info --text="$@"
}


gtk_get_bool() {
  zenity --title "$TITLE" --question --text="$@"
  [ $? = 0 ] && echo "yes" || echo "no"
}

gtk_get_string() {
  zenity --title "$TITLE" --entry --text="$@"
}

gtk_get_choice() {
  # read choices into array
  unset CHOICES
  I=0
  while read line; do
    CHOICES[$((I++))]="FALSE"
    CHOICES[$((I++))]="${line#* }"
  done <<< "$2"
  CHOICES[0]="TRUE"
  CHOICE="`zenity --title "$TITLE" --list --text="$1" --column=" " --column="Choice" --radiolist "${CHOICES[@]}"`"
  grep "$CHOICE" <<< "$2" | cut -f1 -d\ 
}

gtk_download() {
# $1	what to download
# $2	where into
# NOTE: here must be limitation to handle it easily
#       $2 can be dir, if it already exists or if it ends with '/'


  if [ "$2" != "${2%/}" ]; then
    # it has '/' at the end or it is existing directory
    TARGET="$2/${1##*/}"
    [ -d "$2" ] || mkdir -p "$2" || error "Cannot create $2"
  elif [ -d "$2" ]; then
    # it's existing directory
    TARGET="$2/${1##*/}"
  else
    # $2 is file
    TARGET="$2"
  fi

  # download with output to dialog progress bar
  ( wget "$1" -O "$TARGET" -o /dev/stdout | while read I; do
      I="${I//*.......... .......... .......... .......... .......... /}"
      I="${I%%%*}"
      # report changes only
      if [ "$I" ] && [ "$J" != "$I" ]; then
	echo "$I"
	J="$I"
      fi      
    done | zenity --title "$TITLE" --progress --text="Downloading:\n$1\ninto\n$2" --auto-close --auto-kill )
}



detect_dialog() {
  if [ -z "$DISPLAY" ]; then
    if which dialog >/dev/null; then
      tmp=cons
    else
      echo "You want to run without X, but you don't have dialog utility."
      exit 1
    fi
  elif which kdialog > /dev/null; then
    tmp=kde
  elif which zenity > /dev/null; then
    tmp=gtk
  elif which dialog > /dev/null; then
    tmp=cons
  else
    echo "Cannot find working alternative. Both Kdialog and dialog weren't found. Please install and retry."
  fi
  

  error=${tmp}_error
  fatal_error=${tmp}_fatal_error
  info=${tmp}_info
  wait_info=${tmp}_wait_info
  get_bool=${tmp}_get_bool
  get_string=${tmp}_get_string
  get_choice=${tmp}_get_choice
  download=${tmp}_download
}

script_error() {
  error "Congratulations, you found problem in script which cannot be handled. So - what now?
1] feel like winner
2] rerun this script invoking 'sh -x $0 2> /tmp/install_script.log'
3] get to that problem by behaving same way as before
4] send log to sleep_walker@hackndev.com with subject 'install_script.sh error' and attach file /tmp/install_script.log

Thank you!

I'll analyse problem and try to find how to fix it."
}

#####################################################################################
########################### partition and card functions ############################
#####################################################################################
clean_and_create_fat() {
    cat << EOB
o
n
p
1

+${FAT_SIZE}M
t
b
EOB
}

# create EXT partition in rest of card
create_just_ext() {
  cat << EOB
n
p
2


EOB
}

# commands to create EXT partition of given size
create_ext() {
  cat << EOB
n
p
2

+${EXT2_SIZE}M
EOB
}

# commands to create SWAP after creating EXT
create_also_swap() {
  cat << EOB
n
p
3


t
3
82
EOB
}

# write partitioning and exit
finish() {
  echo "w"
}


get_card_size() {
#  CARD_SIZE="`LANG=en fdisk -l "$CARD_DEVICE" 2>/dev/null | grep "$CARD_DEVICE:.*bytes" | cut -f3 -d\ `"
  CARD_BYTE_SIZE="`LANG=en fdisk -l "$CARD_DEVICE" 2>/dev/null | grep "$CARD_DEVICE:.*bytes" | cut -f5 -d\ `"
  CARD_SIZE="$((CARD_BYTE_SIZE / 1024 / 1024))"
}


# now compose action on given data
fdisk_compose_action() {
  clean_and_create_fat
  if is_true "$EXT2_REST"; then
    create_just_ext
  else
    create_ext
    is_true "$USE_SWAP" && create_also_swap
  fi
  finish
}

is_number() {
  grep '^[0-9]\+$' <<< "$1" > /dev/null
}

fdisk_repartition_card() {
  # infinity to pass question at least once :)
  get_card_size
  BAD=yes
  while is_true $BAD; do
    BAD=no
    FAT_SIZE="`$get_string "What should be new size in MB of FAT partition (capacity left - $CARD_SIZE Mb)?\nPlease, keep in mind that it is only partition PalmOS can work with."`"
    [ "$?" = 0 ] || return 1
    is_number "$FAT_SIZE" || { $error "Please entry only number." ; BAD=yes ; }
    [ "$FAT_SIZE" -lt "$CARD_SIZE" ] || { $error "You are attempting to create bigger FAT partition than is capacity of your card" ; BAD=yes ; }
  done

  EXT2_REST="`$get_bool "Should I use rest of card for EXT2 partition?"`"

  if ! is_true $EXT2_REST; then
    BAD=yes
    while is_true $BAD; do
      BAD=no
      EXT2_SIZE="`$get_string "What should be new size in MB of EXT2 partition (capacity left - $((CARD_SIZE - FAT_SIZE)) Mb)?"`"
      [ "$?" = 0 ] || return 1
      is_number "$FAT_SIZE" || { $error "Please entry only number." ; BAD=yes ; }
      [ "$EXT2_SIZE" -lt "$((CARD_SIZE - FAT_SIZE))" ] || { $error "You are attempting to create bigger EXT2 partition than is free space on your card" ; BAD=yes ; }
    done
    
    USE_SWAP="`$get_bool "Do you want swap (capacity left - $((CARD_SIZE - FAT_SIZE - EXT2_SIZE)) Mb)?\nSwap is space, which is used to enlarge memory with disk or card.\nThis is recomended for devices with less RAM than 64 MB and it's useful in general."`"
  fi
  $info "Ensuring that card is not mounted"
  do_unmount_detected_device
  $info "Partitioning card..."
  fdisk_compose_action | fdisk "$CARD_DEVICE" 2> /dev/null > /dev/null || $error "fdisk execution failed"
  $info "Creating FAT filesystem..."
  unset PART
  if grep "mmcblk" <<< "$CARD_DEVICE" > /dev/null; then
    PART=p
  fi
  mkfs.vfat "${CARD_DEVICE}${PART}1" || $error "Creation of FAT filesystem failed!"
  $info "Creating EXT filesystem"
  mkfs.ext2 "${CARD_DEVICE}${PART}2" || $error "Creation of EXT2 filesystem failed!"
  is_true "$USE_SWAP" && $info "Creating swap..." && mkswap "${CARD_DEVICE}${PART}3" || $error "Swap space creation failed!"
  $wait_info "Partitions and filesystems are now successfuly created."
}

do_unmount_detected_device() {
  grep -E "(`find /dev/disk -type l | xargs ls -l | grep -E "(${SHORT_CARD_DEVICE}${PART}[12])$" | tr -s ' ' ' ' | cut -f8 -d\  | tr '\n' '|' | sed 's/|$//'`|${CARD_DEVICE}${PART}[12])" /etc/mtab | cut -f1 -d\  | xargs -i umount {}
}

detect_card_device() {
  TITLE="Card device detection"
  $wait_info "Let's find try to find card device to work with!\nI'll check for changes in /dev/disk/by-id/ which is handled by udev.\nNow please remove your card from card reader.\n\nPress OK when ready"
  $info "Waiting a bit so udev can find out that card is gone"
  sleep 1
  WITHOUT_CARD="`ls /dev/disk/by-id/*`"
  $wait_info "Status without card is now read.\nPlease insert card now. I'll read device list again.\n\nPress OK when ready"
  $info "Waiting a bit so udev can find out that card is there"
  sleep 1
  WITH_CARD="`ls /dev/disk/by-id/*`"
  # sort, show unique files and don't bother with partition devices
  DETECTED_DEVICE="`echo -e "$WITHOUT_CARD\n$WITH_CARD" | sort | uniq -u | grep -v -- '-part[1-9]$'`"
  # sanity check - did I found only one device?
  if [ -z "$DETECTED_DEVICE" ]; then
    $error "I haven't found that device, sorry.\nTry to find it by yourself.\nCard device can be /dev/mmcblkX (for some card readers), /dev/sdX (for others).\nBe careful cause /dev/sdX also match your disk drive."
    return 1
  fi
  if [ `echo "$DETECTED_DEVICE" | wc -l` != 1 ]; then
    $error "I found multiple devices, sorry"
    return 1
  fi

  if is_true `$get_bool "I found this device:\n$DETECTED_DEVICE\n\nDo you want to use it?"`; then
    LONG_CARD_DEVICE="$DETECTED_DEVICE"
  fi
}

#####################################################################################
########################## mount handling functions #################################
#####################################################################################


ask_for_fat_mount() {
  FAT_MOUNT="`$get_string "Where is card FAT partition mounted to?"`"
}

ask_for_ext_mount() {
  EXT2_MOUNT="`$get_string "Where is card EXT2 partition mounted to?"`"
}

lazy_mount() {
  if ! grep "$1" /etc/mtab > /dev/null; then
    # it is not mounted
    mount "$1"
  fi
}

lazy_unmount() {
  if ! grep "$1" /etc/mtab > /dev/null; then
    # it is not mounted
    return
  fi
  umount "$1"
}

#####################################################################################
############################## release functions ####################################
#####################################################################################

parse_links() {
  sed -n 's/.*<a href="\([^"]*\)">.*/\1/p'
}

push_queue() {
  QUEUE="$QUEUE $1"
}

pop_queue() {
  CUR="${QUEUE%% *}"
  if [ "$QUEUE" = "${QUEUE#* }" ]; then
    unset QUEUE
  fi
  QUEUE="${QUEUE#* }"
}

handle_rootfs_image() {
# $1	file to extract

case "$1" in
  *.tar.bz2)
    $info "Extracting $1 into your card, please wait"
    tar xjpf "$1" -C "$EXT2_MOUNT" || $error "An error occured during extraction.\nDo you have enough space on card?" ;;
  *.tar.gz)
    $info "Extracting $1 into your card"
    tar xzpf "$1" -C "$EXT2_MOUNT" || $error "An error occured during extraction.\nDo you have enough space on card?" ;;
  *)
    $error "You have selected image which I can't handle.\nYour choice was:\n$1" ;;
esac
}

fix_root_passwd() {
  # this function will set root password to 'toor'
  sed -i '@^root:@s@^.*$@root:XOY7FA909Ez/w:0:0:root:/home/root:/bin/sh@' "$EXT2_MOUNT/etc/passwd"
}

auryn_images() {
  START="http://auryn.karlin.mff.cuni.cz/oe"
  QUEUE="stable testing latest"
  while [ "$QUEUE" ]; do
    pop_queue
    while read NXT; do
      # don't follow ipk/ dirs
      if [ "$NXT" = "ipk/" ]; then
        break
      # stable images are not separated into device dirs, if I find it, I'm finished
      elif grep 'stable.*images/?$' <<< "$START/$CUR/$NXT" > /dev/null; then
        LINKS="$LINKS $START/$CUR/$NXT"
      # testing and latest are separated into device dirs, I'm finished
      elif grep 'images/[^/]\+' <<< "$START/$CUR/$NXT" > /dev/null; then
        LINKS="$LINKS $START/$CUR/$NXT"
      # no subdir - ommit
      elif [ -z "$NXT" ]; then
        break
      # in other case add subdirectory and push
      else
        push_queue "$CUR/${NXT%/}"
      fi
    done <<< "`wget http://auryn.karlin.mff.cuni.cz/oe/$CUR -o /dev/null -O - | parse_links | grep '/$' | grep -v '\.\./$'`"
  done
  IMAGES="`for dir in $LINKS; do
    wget $dir -o /dev/null -O - | parse_links | grep -v '/$' | sed "s#^#$dir#"
  done`"
  IMAGE_CHOICES="`echo "$IMAGES" | grep -vE "$FILTER" | { i=1; while read image; do echo "$i $image"; i=$((i + 1)); done ; }`"
  IMAGE_NUM="`$get_choice "Which image I should use?" "$IMAGE_CHOICES"`"
  IMAGE="`grep -vE "$FILTER" <<< "$IMAGES" | sed -n ${IMAGE_NUM}p`"
}

do_repartition_wizard() {
#   # repartitioning of card is needed for not "live" releases or loopback releases
  TITLE="3.Repartition, format of card"
  if is_true `$get_bool "Do you want to repartition your card?\n\nALL DATA ON CARD WILL BE LOST!"`; then
    # I need to know device only for partitioning
    # but for that I need to be sure! ;D

    if is_true `$get_bool "Do you know, which device in /dev filesystem represents your card?"`; then
      LONG_CARD_DEVICE="`$get_string "Which device is your SD/MMC card?"`"
      [ -b "$LONG_CARD_DEVICE" ] || { $error "Sorry, the device you entered doesn't exist." ; return 1 ;}
    else
      unset LONG_CARD_DEVICE
      if is_true `$get_bool "Should I try to autodetect your card?"`; then
	detect_card_device || return 1
      fi
      if [ -z "$LONG_CARD_DEVICE" ]; then
	error "Sorry, I can't repartition card if I don't know which device it is"
	return 1
      fi
    fi

    SHORT_CARD_DEVICE="`LANG=en ls -l $LONG_CARD_DEVICE | sed 's#.*/##'`"
    CARD_DEVICE="/dev/$SHORT_CARD_DEVICE"
    [ -b "$CARD_DEVICE" ] || script_error
    do_unmount_detected_device
    fdisk_repartition_card
  elif is_true `$get_bool "Would you like at least recreate new EXT2 filesystem\nYou probably want at least this option when you already have EXT2 partition."`; then
    #to be done
    if is_true `$get_bool "Do you know, which device in /dev filesystem represents your card?"`; then
      LONG_CARD_DEVICE="`$get_string "Which device is your SD/MMC card?"`"
      [ -b "$LONG_CARD_DEVICE" ] || { $error "Sorry, the device you entered doesn't exist." ; return 1 ;}
    else
      unset LONG_CARD_DEVICE
      if is_true `$get_bool "Should I try to autodetect your card?"`; then
	detect_card_device || return 1
      fi
      if [ -z "$LONG_CARD_DEVICE" ]; then
	error "Sorry, I can't repartition card if I don't know which device it is"
	return 1
      fi
    fi

    SHORT_CARD_DEVICE="`LANG=en ls -l $LONG_CARD_DEVICE | sed 's#.*/##'`"
    CARD_DEVICE="/dev/$SHORT_CARD_DEVICE"
    [ -b "$CARD_DEVICE" ] || script_error
    if grep "mmcblk" <<< "$CARD_DEVICE" > /dev/null; then
      PART=p
    fi
    [ -b "${CARD_DEVICE}${PART}2" ] || $fatal_error "Sorry, device ${CARD_DEVICE}${PART}2 is not valid block device.\nThat means that you probably haven't your card partitioned yet."
    do_unmount_detected_device
    mkfs.ext2 "${CARD_DEVICE}${PART}2" || $fatal_error "There was during creating EXT2 filesystem, exiting..."
  fi
  return 0
}

#####################################################################################
############################ main release functions #################################
#####################################################################################

# if you're modifying release function, you can count with:
#	FAT is mounted
#	EXT2 is newly created filesystem and mounted
#	release name transformed tr '-' '_' to be a function name
# ask_and_add_temp_file		will cause question about removal downloaded file
# lazy_download_to_tmp		will first check if it is not already downloaded, ask for reuse and download
# download			wget with GUI, args are url_to_download and where_to, creates also folders if necessary
# auryn_images			choose image on auryn.karlin.mff.cuni.cz and set IMAGE with user's choice


# miska's & snua12's release for T5
m_s_T5_release() {
  if is_true `$get_bool "Do you want to have kernel with 324x484 reslution?"`; then
    HIRES="-hires"
  else
    unset HIRES
  fi

  $download "http://atrey.karlin.mff.cuni.cz/~miska/kernels/tt5$HIRES-kernel.tgz" "$FAT_MOUNT/"
  lazy_download_to_tmp "http://atrey.karlin.mff.cuni.cz/~miska/roots/opie-rootfs-expo-20080505-ext2.tgz"
  tar xzf "/tmp/opie-rootfs-expo-20080505-ext2.tgz" -C "$FAT_MOUNT/"
  ask_and_add_temp_file "/tmp/opie-rootfs-expo-20080505-ext2.tgz"
}

# miska's release for TX
mis_TX_release() {
  if is_true `$get_bool "Do you want to have kernel with 324x484 reslution?"`; then
    HIRES="-hires"
  else
    unset HIRES
  fi

  $download "http://atrey.karlin.mff.cuni.cz/~miska/kernels/tx$HIRES-kernel.tgz" "$FAT_MOUNT/"
  lazy_download_to_tmp "http://atrey.karlin.mff.cuni.cz/~miska/roots/opie-rootfs-expo-20080505-ext2.tgz"
  tar xzf "/tmp/opie-rootfs-expo-20080505-ext2.tgz" -C "$FAT_MOUNT/"
  ask_and_add_temp_file "/tmp/opie-rootfs-expo-20080505-ext2.tgz"
}

# Marex's release for Z71
mx_Z71_release() {
  lazy_download_to_tmp "http://marex.hackndev.com/PalmZ71-BootKit-v0.2-Binary.tar.bz2"
  tar xjpf "/tmp/PalmZ71-BootKit-v0.2-Binary.tar.bz2" Z71Bootkit/part1-vfat -C "$FAT_MOUNT" --strip-components=2
  tar xjpf "/tmp/PalmZ71-BootKit-v0.2-Binary.tar.bz2" Z71Bootkit/part1-ext2 -C "$EXT2_MOUNT" --strip-components=2
  ask_and_add_temp_file "/tmp/PalmZ71-BootKit-v0.2-Binary.tar.bz2"
}

# raster's release for Treo650
rast_T650_release() {
  lazy_download_to_tmp "http://download.enlightenment.org/misc/Illume/Treo-650/$RASTER_RELEASE/sdcard-base.tar.gz"
  tar xzpf "/tmp/sdcard-base.tar.gz" -C "$FAT_MOUNT" --exclude="cocoboot.prc"
  lazy_download_to_tmp "http://download.enlightenment.org/misc/Illume/Treo-650/$RASTER_RELEASE/openmoko-illume-image-glibc-ipk--${RASTER_RELEASE//-/}-palmt650.rootfs.tar.gz"
  handle_rootfs_image "/tmp/openmoko-illume-image-glibc-ipk--${RASTER_RELEASE//-/}-palmt650.rootfs.tar.gz"
  ask_and_add_temp_file "/tmp/openmoko-illume-image-glibc-ipk--${RASTER_RELEASE//-/}-palmt650.rootfs.tar.gz"
  ask_and_add_temp_file "/tmp/sdcard-base.tar.gz"
}

# Alex's Debian Lenny release for Treo650
deb_T650_release() {
  lazy_download_to_tmp "http://releases.hackndev.com/debian-lenny-armel-20081004.rootfs.tar.bz2"
  handle_rootfs_image "/tmp/debian-lenny-armel-20081004.rootfs.tar.bz2"
  $download "http://releases.hackndev.com/palmt650-20081005/zImage" "$FAT_MOUNT/"
  cat << EOB > "$FAT_MOUNT/cocoboot.conf"
cmdline = root=/dev/mmcblk0p2 rootdelay=1
kernel = /zImage
EOB
  ask_and_add_temp_file "/tmp/debian-lenny-armel-20081004.rootfs.tar.bz2"
}

# Marex's Technology Preview 2 for LifeDrive
mx_tp2_LD_release() {
  lazy_download_to_tmp "http://releases.hackndev.com/TP2.tar.bz2"
  # instead of using packed cocoboot I'll download new later instead
  tar xjpf "/tmp/TP2.tar.bz2" -C "$FAT_MOUNT/" --exclude="cocoboot.prc"
  ask_and_add_temp_file "/tmp/TP2.tar.bz2"
}

# kEdAR's release for T3 with Sleep_Walker's kernel
ked_sw_T3_release() {
  LAST_BUILD="`wget "http://sleepwalker.hackndev.com/release/T3/hnd-git/kEdAR/build" `"
  $download "http://sleepwalker.hackndev.com/release/T3/hnd-git/kEdAR/$LAST_BUILD/zImage.T3.sw$LAST_BUILD" "$FAT_MOUNT/"
  $download "http://sleepwalker.hackndev.com/release/T3/hnd-git/kEdAR/$LAST_BUILD/initrd.T3.sw$LAST_BUILD" "$FAT_MOUNT/"
  $download "http://sleepwalker.hackndev.com/release/T3/hnd-git/kEdAR/$LAST_BUILD/modules-T3.sw$LAST_BUILD.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/modlist-OpieMini0719.txt" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/rootfs-OpieMini20070719-xscale.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/konqueror-embedded.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/morefonts_opie.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/dev_tt3.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/kedar_changes.squashfs" "$FAT_MOUNT/linux2ram/"
}

# kEdAR's release for all PXA27x devices
ked_pxa_release() {
  lazy_download_to_tmp "http://kedar.palmlinux.cz/test/k27x/k27x.07.tar.gz"
  # instead of using packed cocoboot I'll download new later instead
  tar xzpf /tmp/k27x.07.tar.gz k27x.07/toCard/ --strip-components=2 -C "$FAT_MOUNT" --exclude="cocoboot-svn1197.prc"
}

# kEdAR's release for T3
ked_T3_release() {
  $download "http://kedar.palmlinux.cz/initrd.$KED_T3_RELEASE.gz" "$FAT_MOUNT/"
  $download "http://kedar.palmlinux.cz/zImage.$KED_T3_RELEASE.gz" "$FAT_MOUNT/"
  $download "http://kedar.palmlinux.cz/cocoboot.conf" "$FAT_MOUNT/"
  $download "http://kedar.palmlinux.cz/linux2ram/modlist-OpieMini0719.txt" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/modules-$KED_T3_RELEASE.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/rootfs-OpieMini20070719-xscale.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/konqueror-embedded.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/morefonts_opie.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/dev_tt3.squashfs" "$FAT_MOUNT/linux2ram/"
  $download "http://kedar.palmlinux.cz/linux2ram/kedar_changes.squashfs" "$FAT_MOUNT/linux2ram/"
}

# Sleep_Walker's kernel and miska's rootfs for Treo680
sw_mis_T680_release() {
  LAST_BUILD="`wget http://sleepwalker.hackndev.com/release/T680/linux-2.6-arm/partition/build -o /dev/null -O -`"
  $download "http://sleepwalker.hackndev.com/release/T680/linux-2.6-arm/partition/$LAST_BUILD/zImage.T680.sw$LAST_BUILD" "$FAT_MOUNT"
  $download "http://sleepwalker.hackndev.com/release/T680/linux-2.6-arm/partition/$LAST_BUILD/cocoboot.conf" "$FAT_MOUNT"
  auryn_images || return
  lazy_download_to_tmp "$IMAGE"
  handle_rootfs_image "/tmp/${IMAGE##*/}"
  lazy_download_to_tmp "http://sleepwalker.hackndev.com/release/T680/linux-2.6-arm/partition/$LAST_BUILD/modules.T680.sw$LAST_BUILD.tar.bz2"
  handle_rootfs_image "/tmp/modules.T680.sw$LAST_BUILD.tar.bz2"
  fix_root_passwd
  ask_and_add_temp_file "/tmp/modules.T680.sw$LAST_BUILD.tar.bz2"
  ask_and_add_temp_file "/tmp/${IMAGE##*/}"
}

# z72ka's release for Z72
z72ka_Z72_release() {
  lazy_download_to_tmp "http://releases.hackndev.com/Angstrom-Opie-PalmZ72-v085.tar.bz2"
  # I'd rather use up to date version of cocoboot
  tar xjpf "/tmp/Angstrom-Opie-PalmZ72-v085.tar.bz2" -C "$FAT_MOUNT" --exclude="cocoboot.prc"
  ask_and_add_temp_file "/tmp/Angstrom-Opie-PalmZ72-v085.tar.bz2"
}

# Marex's (outdated) release for TT
mx_tt_release() {
  lazy_download_to_tmp "http://marex.hackndev.com/PalmTT-BootKit-v0.2-Binary.tar.bz2"
  tar xjpf /tmp/PalmTT-BootKit-v0.2-Binary.tar.bz2 TTBootkit/part1-vfat --strip-components=2 -C "$FAT_MOUNT"
  # I need clean filesystem for extracting this
  lazy_unmount "$EXT2_MOUNT"
  mkfs.ext2 "${CARD_DEVICE}${PART}2"
  mount -t ext2 "${CARD_DEVICE}${PART}2" "$EXT2_MOUNT"
  tar xjpf /tmp/PalmTT-BootKit-v0.2-Binary.tar.bz2 TTBootkit/part2-ext2 --strip-components=2 -C "$EXT2_MOUNT"
  umount "$EXT2_MOUNT"
  ask_and_add_temp_file "PalmTT-BootKit-v0.2-Binary.tar.bz2"
  wait_info "For starting this release please run Garux from your card on Palm.\nEnjoy!"
}

#####################################################################################
########################## general release functions ################################
#####################################################################################

lazy_download_to_tmp() {
  BASENAME="${1##*/}"
  [ -f "/tmp/$BASENAME" ] && is_true `get_bool "Previous download detected.\nShould I reuse it?"` && return
  $download "$1" "/tmp/$BASENAME"
}

do_release_preparations() {
  if is_true `$get_bool "Do you know where are mount points for FAT (and if needed also EXT) partitions?\nYou can have it set in /etc/fstab or you can have it handled by HAL."`; then
    ask_for_fat_mount
    if grep "$RELEASE" <<< "$NEEDS_PARTITION" > /dev/null; then
      ask_for_ext_mount
    fi
    mount "$FAT_MOUNT"
    mount "$EXT2_MOUNT"
  else
  #probably in /media
    if [ -z "$CARD_DEVICE" ]; then
      detect_card_device
    fi
    # now I can have problem, that card is mounted by HAL and I don't know where...
    # let's find all links, which points to detected device partitions
    if grep "mmcblk" <<< "$CARD_DEVICE" > /dev/null; then
      PART=p
    fi
    do_unmount_detected_device
    umount "${CARD_DEVICE}${PART}1"
    umount "${CARD_DEVICE}${PART}2"
    FAT_MOUNT="/mnt/FAT.$$"
    EXT2_MOUNT="/mnt/EXT2.$$"
    mkdir "$FAT_MOUNT"
    mkdir "$EXT2_MOUNT"
    RM_MOUNT_POINTS=true
    mount -t vfat "${CARD_DEVICE}${PART}1" "$FAT_MOUNT"
    mount -t ext2 "${CARD_DEVICE}${PART}2" "$EXT2_MOUNT"
  fi
}

do_cocoboot() {
  $download "http://hackndev.com/trac/attachment/wiki/Bootpacks/cocoboot.prc" "$FAT_MOUNT/palm/Launcher"
}

do_wizard() {
  TITLE="Welcome..."
  $wait_info "Welcome to BFU friendly installer!
This will take you through simple installation process...
Steps to be done:
1] select device
2] select release
3] repartition and/or create filesystem if needed
4] download and extract or copy needed release related files
5] download cocoboot and create cocoboot.conf
"
  TITLE="1.Device selection"
  DEVICE="`$get_choice "Select your device" "$DEVICE_LIST"`"
  TITLE="2.Release selection"
  STRIPPED_LIST="`sed -n "/^$DEVICE/s/^$DEVICE //p" <<< "$RELEASE_LIST"`"
  RELEASE="`$get_choice "Select release to install" "$STRIPPED_LIST"`"
  if grep "$RELEASE" <<< "$NEEDS_PARTITION" > /dev/null; then
    do_repartition_wizard || return 1
  fi
  TITLE="4.Download and install"
  # now I'll mount partitions I need
  do_release_preparations
  "${RELEASE//-/_}_release"
  TITLE="5.Cocoboot"
  if ! grep "$RELEASE" <<< "$NOT_COCOBOOT" > /dev/null || is_true "$OMMIT_COCOBOOT"; then
    do_cocoboot
  fi
  clean_work
  TITLE="Installation complete"
  $wait_info "Congratulations, installation is now complete.\nYou may remove your card now."
}

clean_work() {
  # run cleaning task only once
  is_true $CLEAN_WORK_DONE && return 0
  lazy_unmount "$FAT_MOUNT"
  lazy_unmount "$EXT2_MOUNT"
  is_true "$RM_MOUNT_POINTS" && rmdir "$FAT_MOUNT" "$EXT2_MOUNT"
  if [ "$TEMP_COUNT" ]; then
    rm "${TEMP_FILES[@]}"
  fi
  CLEAN_WORK_DONE="true"
}


if [ "$0" != "/bin/bash" ] && [ "$0" != "bash" ]; then
  # I'm run, not sourced
  detect_dialog
  only_root_pass
  do_wizard
  clean_work
else
  echo "Sourcing..."
fi

