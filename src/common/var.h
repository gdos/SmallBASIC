// This file is part of SmallBASIC
//
// SmallBASIC Code & Variable Manager.
//
// This program is distributed under the terms of the GPL v2.0 or later
// Download the GNU Public License (GPL) from www.gnu.org
//
// Copyright(C) 2000 Nicholas Christopoulos

/**
 * @defgroup var Variables
 */
/**
 * @defgroup exec Executor
 */

#if !defined(_sb_cvm_h)
#define _sb_cvm_h

#include "common/sys.h"

#ifdef V_INT
#undef V_INT
#endif

#ifdef V_ARRAY
#undef V_ARRAY
#endif

/*
 * Variable - types
 */
#define V_INT       0 /**< variable type, 32bit integer                @ingroup var */
#define V_NUM       1 /**< variable type, 64bit float (same as V_NUM)  @ingroup var */
#define V_STR       2 /**< variable type, string                       @ingroup var */
#define V_ARRAY     3 /**< variable type, array of variables           @ingroup var */
#define V_PTR       4 /**< variable type, pointer to UDF or label      @ingroup var */
#define V_MAP       5 /**< variable type, associative array            @ingroup var */
#define V_REF       6 /**< variable type, reference another var        @ingroup var */
#define V_FUNC      7 /**< variable type, object method                @ingroup var */
#define V_NIL       8 /**< variable type, null value                   @ingroup var */

/*
 *   predefined system variables - index
 */
#define SYSVAR_SBVER        0  /**< system variable, SBVER     @ingroup var */
#define SYSVAR_PI           1  /**< system variable, PI        @ingroup var */
#define SYSVAR_XMAX         2  /**< system variable, XMAX      @ingroup var */
#define SYSVAR_YMAX         3  /**< system variable, YMAX      @ingroup var */
#define SYSVAR_TRUE         4  /**< system variable, TRUE      @ingroup var */
#define SYSVAR_FALSE        5  /**< system variable, FALSE     @ingroup var */
#define SYSVAR_CWD          6  /**< system variable, CWD$      @ingroup var */
#define SYSVAR_HOME         7  /**< system variable, HOME$     @ingroup var */
#define SYSVAR_COMMAND      8  /**< system variable, COMMAND$  @ingroup var */
#define SYSVAR_X            9  /**< system variable, X         @ingroup var */
#define SYSVAR_Y            10 /**< system variable, Y         @ingroup var */
#define SYSVAR_SELF         11 /**< system variable, SELF      @ingroup var */
#define SYSVAR_NONE         12 /**< system variable, NONE      @ingroup var */
#define SYSVAR_MAXINT       13 /**< system variable, INTMAX    @ingroup var */
#define SYSVAR_COUNT        14

/**
 * @ingroup var
 * @def MAXDIM Maxium number of array-dimensions
 */
#define MAXDIM 2

#if defined(__cplusplus)
extern "C" {
#endif

struct var_s;
typedef void (*method) (struct var_s *self);

/**
 * @ingroup var
 * @typedef var_s
 *
 * VARIANT DATA TYPE
 */
typedef struct var_s {
  // value
  union {
    var_num_t n; /**< numeric value */
    var_int_t i; /**< integer value */

    // pointer to sub/func variable
    struct {
      bcip_t p; /** address pointer */
      bcip_t v; /** return-var ID */
    } ap;

    // associative array/map
    struct {
      void *map; /** pointer the map structure */
      uint32_t count;
      uint32_t size;
    } m;

    // reference variable
    struct var_s *ref;

    // object method
    struct {
      method cb;
    } fn;

    // generic ptr (string)
    struct {
      char *ptr; /**< data ptr (possibly, string pointer) */
      uint32_t length; /**< the string length */
      byte owner;
    } p;

    // array
    struct {
      struct var_s *data; /**< array data pointer */
      uint32_t size; /**< the number of elements */
      uint32_t capacity; /**< the number of slots */
      int32_t lbound[MAXDIM]; /**< lower bound */
      int32_t ubound[MAXDIM]; /**< upper bound */
      byte maxdim; /**< number of dimensions */
    } a;

    // next item in the free-list
    struct var_s *pool_next;
  } v;

  byte type; /**< variable's type */
  byte const_flag; /**< non-zero if constants */
  byte pooled; /** whether held in pooled memory */
} var_t;

typedef var_t *var_p_t;

/*
 * label
 */
typedef struct lab_s {
  bcip_t ip;
} lab_t;

/**
 * @ingroup exec
 * @struct stknode_s
 *
 * EXECUTOR's STACK NODE
 */
typedef struct stknode_s {
  union {
    /**
     *  FOR-TO-NEXT
     */
    struct {
      var_t *var_ptr; /**< 'FOR' variable */
      var_t *arr_ptr; /**< FOR-IN array-variable */
      bcip_t to_expr_ip; /**< IP of 'TO' expression */
      bcip_t step_expr_ip; /**< IP of 'STEP' expression (FOR-IN = current element) */
      bcip_t jump_ip; /**< code block IP */
      bcip_t exit_ip; /**< EXIT command IP to go */
      code_t subtype; /**< kwTO | kwIN */
      byte flags; /**< ... */
    } vfor;

    /**
     *  REPEAT/WHILE
     */
    struct {
      bcip_t exit_ip; /**< EXIT command IP to go */
    } vloop;

    /**
     *  IF/ELIF
     */
    struct {
      bcip_t lcond; /**< result of the last condition */
    } vif;

    /**
     *  SELECT CASE
     */
    struct {
      var_t *var_ptr;
      byte flags;
    } vcase;

    /**
     *  GOSUB
     */
    struct {
      bcip_t ret_ip; /**< return ip */
    } vgosub;

    /**
     *  CALL UDP/F
     */
    struct {
      var_t *retvar;   /**< return-variable data */
      bcip_t ret_ip;   /**< return ip */
      bid_t rvid;      /**< return-variable ID */
      int task_id; /**< task_id or -1 (this task) */
      uint16_t pcount; /**< number of parameters */
    } vcall;

    /**
     *  Create dynamic variable (LOCAL or PARAMETER)
     */
    struct {
      var_t *vptr; /**< previous variable */
      bid_t vid; /**< variable index in tvar */
    } vdvar;

    /**
     *  parameter (CALL UDP/F)
     */
    struct {
      var_t *res; /**< variable pointer (for BYVAL this is a clone) */
      uint16_t vcheck; /**< checks (1=BYVAL ONLY, 3=BYVAL|BYREF, 2=BYREF ONLY) */
    } param;

    /**
     * try/catch
     */
    struct {
      bcip_t catch_ip;
    } vtry;

    struct {
      var_t *catch_var;
    } vcatch;
  } x;

  int line; /** line number of current execution **/
  code_t type; /**< type of node (keyword id, i.e. kwGOSUB, kwFOR, etc) */
} stknode_t;

/*
 * Basic variable's API
 */

/**
 * @ingroup var
 *
 * intialises the var pool
 */
void v_init_pool(void);

/**
 * @ingroup var
 *
 * free pooled var
 */
void v_pool_free(var_t *var);

/**
 * @ingroup var
 *
 * creates a new variable
 *
 * @return a newly created var_t object
 */
var_t *v_new(void);

/**
 * @ingroup var
 *
 * creates a new variable array
 *
 * @return a newly created var_t array of the given size
 */
void v_new_array(var_t *var, unsigned size);

/**
 * @ingroup var
 *
 * frees memory associated with the given array
 */
void v_array_free(var_t *var);

/**
 * @ingroup var
 *
 * initialise the variable as a string of the given length
 */
void v_init_str(var_t *var, int length);

/**
 * @ingroup var
 *
 * takes ownership of the given allocated string
 */
void v_move_str(var_t *var, char *str);

/**
 * @ingroup var
 *
 * returns true if the value is not 0/NULL
 *
 * @param v the variable
 * @return true if the value is not 0/NULL
 */
int v_is_nonzero(var_t *v);

/**
 * @ingroup var
 *
 * compares two variables
 *
 * @param a the left-side variable
 * @param b the right-side variable
 * @return 0 if a = b, <0 if a < b, >0 if a > b
 */
int v_compare(var_t *a, var_t *b);

/**
 * @ingroup var
 *
 * calculates the result type of the addition of two variables
 *
 * @param a the left-side variable
 * @param b the right-side variable
 * @return the type of the new variable
 */
int v_addtype(var_t *a, var_t *b);

/**
 * @ingroup var
 *
 * adds two variables
 *
 * @param result the result
 * @param a the left-side variable
 * @param b the right-side variable
 */
void v_add(var_t *result, var_t *a, var_t *b);

/**
 * @ingroup var
 *
 * assigning: dest = src
 *
 * @param dest the destination-var
 * @param src the source-var
 */
void v_set(var_t *dest, const var_t *src);

/**
 * @ingroup var
 *
 * assigning: dest = src
 *
 * @param dest the destination-var
 * @param src the source-var
 */
void v_move(var_t *dest, const var_t *src);

/**
 * @ingroup var
 *
 * increase the value of variable a by b
 * (similar to v_add())
 *
 * @param a is the variable
 * @param b is the increment
 */
void v_inc(var_t *a, var_t *b);

/**
 * @ingroup var
 *
 * returns the sign of a variable
 *
 * @param x the variable
 * @return the sign
 */
int v_sign(var_t *x);

/**
 * @ingroup var
 *
 * returns the var_t pointer of an array element
 *
 * @param v is the array-variable
 * @param index is the element's index number
 * @return the var_t pointer of an array element
 */
var_t *v_getelemptr(var_t *v, uint32_t index);

/**
 * @ingroup var
 *
 * create a string variable (with value str)
 *
 * @param v is the variable
 * @param src is the string
 */
void v_createstr(var_t *v, const char *src);

/**
 * @ingroup var
 *
 * print variable as string
 *
 * @param arg is the variable
 */
char *v_str(var_t *arg);

/**
 * @ingroup var
 *
 * convert variable to string
 *
 * @param arg is the variable
 */
void v_tostr(var_t *arg);

/**
 * @ingroup var
 *
 * copies data from one user defined structure to another
 */
void v_set_uds(bcip_t dst_ip, bcip_t src_ip);

/**
 * @ingroup var
 *
 * clones data from one user defined structure to another. pushes
 * replaced variables onto the stack for later clean
 *
 */
void v_clone_uds(bcip_t dst_ip, bcip_t src_ip);

/*
 * returns the starting address for the uds of the given id
 */
bcip_t v_get_uds_ip(bcip_t var_id);

/**
 * @ingroup var
 *
 * creates a new variable which is a clone of 'source'.
 *
 * @param source is the source
 * @return a newly created var_t object, clone of 'source'
 */
var_t *v_clone(const var_t *source);

/**
 * @ingroup var
 *
 * resizes an array-variable to 1-dimension array of 'size' elements
 *
 * @param v the variable
 * @param size the number of the elements
 */
void v_resize_array(var_t *v, uint32_t size);

/**
 * @ingroup var
 *
 * convert variable v to a RxC matrix
 *
 * @param v the variable
 * @param r the number of the rows
 * @param c the number of the columns
 */
void v_tomatrix(var_t *v, int r, int c);

/**
 * @ingroup var
 *
 * converts the variable v to an array of R elements.
 * R can be zero for zero-length arrays
 *
 * @param v the variable
 * @param r the number of the elements
 */
void v_toarray1(var_t *v, uint32_t r);

/**
 * @ingroup var
 *
 * returns true if the 'v' is empty (see EMPTY())
 *
 * @param v the variable
 * @return non-zero if v is not 'empty'
 */
int v_isempty(var_t *v);

/**
 * @ingroup var
 *
 * returns the length of the variable (see LEN())
 *
 * @param v the variable
 * @return the length of the variable
 */
int v_length(var_t *v);

/**
 * @ingroup var
 * @page var_12_2001 Var API (Dec 2001)
 *
 * @code
 * Use these routines
 *
 * Memory free/alloc is contolled inside these functions
 * The only thing that you must care of, is when you declare local var_t elements
 *
 * Auto-type-convertion is controlled inside these functions,
 * So if you want a string value of an integer you just do strcpy(buf,v_getstr(&myvar));
 * or a numeric value of a string R = v_getnum(&myvar);
 *
 * Using variables on code:
 *
 * void myfunc() {    // using them in stack
 *  var_t    myvar;
 *  v_init(&myvar);  // DO NOT FORGET THIS! local variables are had random data
 *  ...
 *  v_setstr(&myvar, "Hello, world");
 *  ...
 *  v_set(&myvar, &another_var); // copy variables (LET myvar = another_var)
 *  ...
 *  v_setint(&myvar, 0x100);     // Variable will be cleared automatically
 *  ...
 *  v_free(&myvar);
 * }
 *
 * void myfunc() {                   // using dynamic memory
 *  var_t *myvar_p;
 *
 *  myvar_p = v_new();               //  create a new variable
 *  ...
 *  v_setstr(myvar_p, "Hello, world");
 *  ...
 *  v_setint(myvar_p, 0x100);        // Variable will be cleared automatically
 *  ...
 *  v_free(myvar_p);             // clear variable's data
 *  v_detach(myvar_p);               // free the variable
 * }
 *
 * @endcode
 */

/**
 * @ingroup var
 *
 * sets a string value to variable 'var'
 *
 * @param var is the variable
 * @param string is the string
 */
void v_setstr(var_t *var, const char *string);

/**
 * @ingroup var
 *
 * sets a string value to variable 'var' to the given length
 *
 * @param var is the variable
 * @param string is the string
 */
void v_setstrn(var_t *var, const char *string, int len);

/**
 * @ingroup var
 *
 * concate string to variable 'var'
 *
 * @param var is the variable
 * @param string is the string
 */
void v_strcat(var_t *var, const char *string);

/**
 * @ingroup var
 *
 * sets a real-number value to variable 'var'
 *
 * @param var is the variable
 * @param number is the number
 */
void v_setreal(var_t *var, var_num_t number);

/**
 * @ingroup var
 *
 * sets an integer value to variable 'var'
 *
 * @param var is the variable
 * @param integer is the integer
 */
void v_setint(var_t *var, var_int_t integer);

/**
 * @ingroup var
 *
 * makes 'var' an empty string variable
 *
 * @param var is the variable
 */
void v_zerostr(var_t *var);

/**
 * @ingroup var
 *
 * assign value 'str' to var. the final type of the var will be decided
 * on that function (numeric if the str is a numeric-constant string or string)
 *
 * @note used by INPUT to convert the variables
 *
 * @param str is the string
 * @param var is the variable
 */
void v_input2var(const char *str, var_t *var);

/**
 *< returns the var_t pointer of the element i
 * on the array x. i is a zero-based, one dim, index.
 * @ingroup var
*/
#define v_elem(var, i) &((var)->v.a.data[i])

/**
 * < the number of the elements of the array (x)
 * @ingroup var
 */
#define v_asize(x)      ((x)->v.a.size)

/**
 * < returns the integer value of variable v
 * @ingroup var
 */
#define v_getint(v)  v_igetval((v))

/**
 * < returns the real value of variable v
 * @ingroup var
 */
#define v_getreal(v)  v_getval((v))

/**
 * @ingroup var
 *
 * returns the string-pointer of variable v
 *
 * @param v is the variable
 * @return the pointer of the string
 */
char *v_getstr(var_t *v);

/**
 * @ingroup var
 *
 * returns the length of the var string
 *
 * @param v is the variable
 * @return the string length
 */
int v_strlen(const var_t *v);

/**
 * @ingroup var
 *
 * returns whether the variable is of the given type
 *
 * @param v is the variable
 * @return whether the variable is of the given type
 */
#define v_is_type(v, t) (v != NULL && v->type == t)

/**
 * @ingroup var
 *
 * creates an system image object
 *
 * @param v is the variable
 */
void v_create_image(var_p_t var);

/**
 * @ingroup var
 *
 * creates an system form object
 *
 * @param v is the variable
 */
void v_create_form(var_p_t var);

/**
 * @ingroup var
 *
 * creates an system window object
 *
 * @param v is the variable
 */
void v_create_window(var_p_t var);

void code_jump_label(uint16_t label_id);  // IP <- LABEL_IP_TABLE[label_id]

#define code_jump(newip) prog_ip=(newip) /**< IP <- NewIP @ingroup exec */

/**
 * @ingroup exec
 *
 * stores a node to stack
 *
 * @param node the stack node
 */
stknode_t *code_push(code_t type);

/**
 * @ingroup exec
 *
 * restores the topmost node from stack
 *
 * @param node the stack node
 */
void code_pop(stknode_t *node, int expected_type);

/**
 * @ingroup exec
 *
 * POPs and frees the topmost node from stack and returns the node type
 *
 */
int code_pop_and_free();

/**
 * @ingroup exec
 *
 * returns the node at the top of the stack. does not change the stack.
 *
 */
stknode_t *code_stackpeek();

#define code_peek()         prog_source[prog_ip]    /**< R(byte) <- Code[IP]          @ingroup exec */
#define code_getnext()      prog_source[prog_ip++]  /**< R(byte) <- Code[IP]; IP ++;  @ingroup exec */

#define code_skipnext()     prog_ip++   /**< IP ++;   @ingroup exec */
#define code_skipnext16()   prog_ip+=2  /**< IP += 2; @ingroup exec */
#define code_skipnext32()   prog_ip+=4  /**< IP += 4; @ingroup exec */
#define code_skipnext64()   prog_ip+=8  /**< IP += 8; @ingroup exec */

#if defined(CPU_BIGENDIAN)
#define code_getnext16()    (prog_ip+=2, (prog_source[prog_ip-2]<<8)|prog_source[prog_ip-1])
#define code_peeknext16()   ((prog_source[prog_ip]<<8)|prog_source[prog_ip+1])
#define code_peek16(o)      ((prog_source[(o)]<<8)|prog_source[(o)+1])
#define code_peek32(o)      (((bcip_t)code_peek16((o)) << 16) + (bcip_t)code_peek16((o)+2))
#else
#define code_getnext16()    (*((uint16_t *)(prog_source+(prog_ip+=2)-2)))
#define code_peeknext16()   (*((uint16_t *)(prog_source+prog_ip)))
#define code_peek16(o)      (*((uint16_t *)(prog_source+(o))))
#define code_peek32(o)      (*((uint32_t *)(prog_source+(o))))
#endif

#define code_skipopr()   code_skipnext16()    /**< skip operator  @ingroup exec */
#define code_skipsep()   code_skipnext16()    /**< skip separator @ingroup exec */
  /**< returns the separator and advance (IP) to next command @ingroup exec */
#define code_getsep()    (prog_ip ++, prog_source[prog_ip++])
#define code_peeksep()   (prog_source[prog_ip+1])

#define code_getaddr()   code_getnext32()  /**< get address value and advance        @ingroup exec */
#define code_skipaddr()  code_skipnext32() /**< skip address field                   @ingroup exec */
#define code_getstrlen() code_getnext32()  /**< get strlen (kwTYPE_STR) and advance  @ingroup exec */
#define code_peekaddr(i) code_peek32((i))  /**< peek address field at offset i       @ingroup exec */

/**
 * @ingroup var
 * @page sysvar System variables
 * @code
 * System variables (osname, osver, bpp, xmax, etc)
 *
 * The variables must be defined
 * a) here (see in first lines) - (variable's index)
 * b) in scan.c (variable's name)
 * c) in brun.c or device.c (variable's value)
 *
 * DO NOT LOSE THE ORDER
 * @endcode
 */

/**
 * @ingroup var
 *
 * sets an integer value to a system variable (constant)
 *
 * @param index is the system variable's index
 * @param val the value
 */
void setsysvar_int(int index, var_int_t val);

/**
 * @ingroup var
 *
 * sets a double value to a system variable (constant)
 *
 * @param index is the system variable's index
 * @param val the value
 */
void setsysvar_num(int index, var_num_t val);

/**
 * @ingroup var
 *
 * sets a string value to a system variable (constant)
 *
 * @param index is the system variable's index
 * @param val the value
 */
void setsysvar_str(int index, const char *value);

/*
 * in eval.c
 */
var_num_t *mat_toc(var_t *v, int32_t *rows, int32_t *cols);

void mat_tov(var_t *v, var_num_t *m, int32_t rows, int32_t cols,
             int protect_col1);

#if defined(__cplusplus)
}
#endif

#endif
