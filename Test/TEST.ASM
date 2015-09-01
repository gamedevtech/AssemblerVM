
;-----------------------------------------------------------------------------
; Declare external C functions
;-----------------------------------------------------------------------------

        ClearScreen             f       1
        DrawObject              f       2
        SwapScreen              f       3
        SetObjectRotation       f       4
        SetObjectPosition       f       5
        sin                     f       6
        cos                     f       7
        Open3DView              f       8
        GetKeyboardCode         f       9

;-----------------------------------------------------------------------------
; Declare global variables
;-----------------------------------------------------------------------------

        position_x      v       0
        position_y      v       0
        rotation        v       0
        key_code        v       0

;-----------------------------------------------------------------------------
; CODE - Start
;-----------------------------------------------------------------------------

        ; Initialise OpenGL

        callc   Open3DView

logic_loop:

        call    UpdateScreen
        call    HandleKeyboard

        ; Check to see if the escape key is pressed

        push    [key_code]
        push    16
        and
        push    16
        je      exit_unit

        ; Change object parameters

        push    [rotation]
        callc   SetObjectRotation

        push    [position_y]
        push    [position_x]
        callc   SetObjectPosition

        jmp     logic_loop

exit_unit:

        exit


;--- UpdateScreen ------------------------------------------------------------


UpdateScreen:

        callc   ClearScreen
        callc   DrawObject
        callc   SwapScreen

        ret


;--- HandleKeyboard ----------------------------------------------------------


HandleKeyboard:

        callc   GetKeyboardCode
        pop     [key_code]

        ; Check to see if the left key is being held down

        push    [key_code]
        push    4
        and
        push    0
        je      no_left

        push    [rotation]
        push    32768
        add
        pop     [rotation]

no_left:

        ; Check to see if the right key is being held down

        push    [key_code]
        push    8
        and
        push    0
        je      no_right

        push    [rotation]
        push    32768
        sub
        pop     [rotation]

no_right:

        ; Check to see if the up key is being held down

        push    [key_code]
        push    1
        and
        push    0
        je      no_up

        push    [position_x]
        push    [rotation]
        callc   sin
        push    200
        div
        sub
        pop     [position_x]

        push    [position_y]
        push    [rotation]
        callc   cos
        push    200
        div
        add
        pop     [position_y]

no_up:

        ; Check to see if the down key is being held down

        push    [key_code]
        push    2
        and
        push    0
        je      no_down

        push    [position_x]
        push    [rotation]
        callc   sin
        push    200
        div
        add
        pop     [position_x]

        push    [position_y]
        push    [rotation]
        callc   cos
        push    200
        div
        sub
        pop     [position_y]

no_down:

        ret

;-----------------------------------------------------------------------------
; END
;-----------------------------------------------------------------------------


