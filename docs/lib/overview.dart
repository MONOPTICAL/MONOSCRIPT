import 'package:flutter/material.dart';
import 'package:syntax_highlight/syntax_highlight.dart';
import 'dart:js' as js;

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'MONOSCRIPT Docs',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        primarySwatch: Colors.blueGrey,
        brightness: Brightness.dark,
        fontFamily: 'Inter',
      ),
      home: MonoScriptPage(),
    );
  }
}

// Класс для хранения примеров кода
class CodeExample {
  final String name;
  final String code;
  final String description;
  
  CodeExample({required this.name, required this.code, this.description = ''});
}

class MonoScriptPage extends StatefulWidget {
  @override
  _MonoScriptPageState createState() => _MonoScriptPageState();
}

class _MonoScriptPageState extends State<MonoScriptPage> {

final List<CodeExample> examples = [
  CodeExample(
    name: 'Fibonacci',
    description: 'Classic Fibonacci number calculation algorithm',
    code: '''[i64]fibonacci(i64: n) @strict @pure
|   if (n == 0)
|   |   return 0
|   else if (n == 1)
|   |   return 1
|   return fibonacci(n - 1->i64) + fibonacci(n - 2->i64)

[i32]main()
|   echo(toString_long(fibonacci(47))) // 2971215073
|   return 0''',
  ),
  CodeExample(
    name: 'Hello World',
    description: 'Simple text output example',
    code: '''[void]hello_world() @strict @entry
|   echo("Hello World!!!")''',
  ),
  CodeExample(
    name: 'Sort',
    description: 'Implementation of bubble sort algorithm',
    code: '''[void]bubbleSort(array<i32>: arr)
|   n ^= len(arr)
|   for i in range(0, n)
|   |   for j in range(0, n - 1)
|   |   |   if arr[j] > arr[j + 1]
|   |   |   |   temp ^= arr[j]
|   |   |   |   arr[j] = arr[j + 1]
|   |   |   |   arr[j + 1] = temp

[string]getArray(array<i32>: arr)
|   result ^= ""
|   for i in range(0, len(arr))
|   |   result += arr[i]
|   return result

[void]main()
|   array<i32> nums = [5, 3, 1, 4, 2]
|   echo("Before sort:" + getArray(nums))
|
|   bubbleSort(nums)
|
|   echo("After sort:" + getArray(nums))''',
  ),
  CodeExample(
    name: 'General Code',
    description: 'Example of string manipulation and type conversion',
    code: '''[string]hello_str() @strict
|   string mark = " hz"
|   mark = "x" + mark + toString_int(2)
|   return "Hello" + mark

[i32]main() @entry
|   string concatString = hello_str() + " " + "World" + "!"
|   echo(concatString)
|   i1 isTrue = ((3>2)!=true)==0 // 1
|   i32 castedValue = -isTrue + 5 // -1 + 5
|   return castedValue // 4''',
  ),
  CodeExample(
    name: 'Modules',
    description: 'Example of module usage',
    code: '''// math.ms
[i32]mySum(i32 : a, i32 : b)
|   return a + b

[i32]myMul(i32 : a, i32 : b)
|   return a * b

// moduleTest.ms
use
|>  math

[i32]moduleTest() @entry
|   // Test the math module
|   final a ^= 5
|   final b ^= 10
|   const i32 sum = mySum(a, b)
|   const i32 mul = myMul(a, b)
|   // Print the results
|   echo("a: " + toString_int(a) + ", b: " + toString_int(b))
|   echo("Sum: " + toString_int(sum) + ", Mul: " + toString_int(mul))
|   return 0'''
  )
];
  
  CodeExample? selectedExample;
  Highlighter? highlighter;

  @override
  void initState() {
    super.initState();
    selectedExample = examples[0];
    _initializeHighlighter();
  }
  
  Future<void> _initializeHighlighter() async {
    await Highlighter.initialize(['dart']); 
    var darkTheme = await HighlighterTheme.loadDarkTheme(); 
    setState(() {
      highlighter = Highlighter(
        language: 'dart', 
        theme: darkTheme,
      );
    });
  }
  
  Widget _getHighlightedCode(String code) {
    if (highlighter != null) {
      return Text.rich(highlighter!.highlight(code));
    } else {
      return Text(
        code,
        style: TextStyle(
          fontFamily: 'Fira Code',
          fontSize: 14,
          height: 1.5,
          color: Colors.white,
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Color(0xFF0D1321),
      appBar: AppBar(
        backgroundColor: Colors.white,
        titleSpacing: 5,
        toolbarHeight: 56,
        title: Row(
          mainAxisAlignment: MainAxisAlignment.start,
          children: <Widget>[
            SizedBox(width: 67.5),
            Image(
              image: AssetImage('images/MONOSCRIPT_white.png'),
              height: 60,
              width: 235,
            ),
            Expanded(
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: <Widget>[
                  _navBarItem('Overview', isActive: true),
                  _navBarItem('Get Started'),
                  _navBarItem('Docs'),
                  _navBarItem('Community'),
                  _navBarItem('Blog'),
                ],
              ),
            ),
          ],
        ),
      ),
      body: SingleChildScrollView(
        child: IntrinsicHeight(
          child: Column(
            children: <Widget>[
              // Основной контент
              _welcomeContent(),
              _smallOverview(),

              // Дополнительное пространство
              SizedBox(height: 40),

              // Нижняя хуйня
              _footer(),
            ],
          ),
        ),
      ),
    );
  }

Widget _welcomeContent() {
  return SingleChildScrollView(
    child: Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        // Основная часть
        Expanded(
          flex: 3,
          child: Container(
            margin: EdgeInsets.only(right: 16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Заголовок и описание
                Padding(
                  padding: EdgeInsets.fromLTRB(80.0, 48.0, 24.0, 0.0),
                  child: Row(  // Row для размещения заголовка и логотипа рядом
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      // Левая часть с заголовком
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: <Widget>[
                            _buildTitle(),
                            SizedBox(height: 16),
                            Text(
                              '"Secure, readable plugin language: Kubernetes, security, automation scripts"',
                              style: TextStyle(
                                fontSize: 16,
                                fontStyle: FontStyle.italic,
                                color: Colors.white70,
                              ),
                            ),
                            SizedBox(height: 24),
                            _buildButtonRow(),
                            SizedBox(height: 40),
                            
                            // Секция с примерами кода
                            Text(
                              'Code Examples',
                              style: TextStyle(
                                fontSize: 22,
                                fontWeight: FontWeight.bold,
                                color: Colors.white,
                              ),
                            ),
                            SizedBox(height: 8),
                            
                            // Выбор примеров кода
                            _buildExampleChips(),
                          ],
                        ),
                      ),
                      
                      // Логотип и текст MONOPTICAL справа
                      Container(
                        width: 450,
                        padding: EdgeInsets.fromLTRB(0, 0, 48, 0),
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Container(
                              padding: EdgeInsets.symmetric(horizontal: 12, vertical: 6),
                              decoration: BoxDecoration(
                                color: Colors.white,
                                borderRadius: BorderRadius.circular(6),
                                border: Border.all(
                                  color: Colors.black,
                                  width: 1,
                                ),
                              ),
                              child: Row(
                                mainAxisSize: MainAxisSize.min,
                                children: [
                                  Text(
                                    'By ',
                                    style: TextStyle(
                                      fontSize: 16,
                                      fontWeight: FontWeight.w300,
                                      color: Colors.black54,
                                      letterSpacing: 0.5,
                                    ),
                                  ),
                                  Text(
                                    'MONOPTICAL',
                                    style: TextStyle(
                                      fontSize: 18,
                                      fontWeight: FontWeight.bold,
                                      color: const Color.fromARGB(255, 61, 58, 54),
                                      letterSpacing: 0.8,
                                    ),
                                  ),
                                  SizedBox(width: 6),
                                  Icon(Icons.verified, size: 16, color: Colors.black54)
                                ],
                              ),
                            ),
                            SizedBox(height: 8),
                            Container(
                              width: 450,
                              height: 250,
                              decoration: BoxDecoration(
                                borderRadius: BorderRadius.circular(16),
                                color: Colors.white,
                                boxShadow: [
                                  BoxShadow(
                                    color: Colors.black26,
                                    blurRadius: 10,
                                    spreadRadius: 2,
                                    offset: Offset(0, 4),
                                  ),
                                ],
                              ),
                              child: ClipRRect(
                                borderRadius: BorderRadius.circular(16),
                                child: Image(
                                  image: AssetImage("images/FullLogo.png"),
                                  fit: BoxFit.cover,
                                ),
                              ),
                            ),
                            SizedBox(height: 16),
                          ],
                        ),
                      ),
                    ],
                  ),
                ),
                
                // Секция примера кода 
                Padding(
                  padding: EdgeInsets.fromLTRB(80.0, 24.0, 24.0, 16.0),
                  child: selectedExample != null && selectedExample!.description.isNotEmpty
                    ? Text(
                        selectedExample!.description,
                        style: TextStyle(
                          fontSize: 16,
                          fontStyle: FontStyle.italic,
                          color: Colors.white70,
                        ),
                      )
                    : SizedBox(),
                ),
                
                // Блок кода с фиксированной высотой
                Padding(
                  padding: EdgeInsets.fromLTRB(80.0, 0.0, 24.0, 24.0),
                  child: Row(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      // Блок кода (слева)
                      Expanded(
                        flex: 7, // 70% места для кода
                        child: selectedExample != null
                          ? Container(
                              height: 750, // Фиксированная высота
                              decoration: BoxDecoration(
                                color: Color(0xFF0D1117),
                                borderRadius: BorderRadius.circular(8),
                                border: Border.all(color: Color(0xFF30363D)),
                                boxShadow: [
                                  BoxShadow(
                                    color: Colors.black.withOpacity(0.3),
                                    blurRadius: 8,
                                    offset: Offset(0, 2),
                                  ),
                                ],
                              ),
                              child: SingleChildScrollView(
                                padding: EdgeInsets.all(16),
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    // Имитация строки с кнопками окна терминала
                                    Row(
                                      mainAxisSize: MainAxisSize.min,
                                      children: [
                                        Container(height: 12, width: 12, decoration: BoxDecoration(color: Colors.red, borderRadius: BorderRadius.circular(6))),
                                        SizedBox(width: 6),
                                        Container(height: 12, width: 12, decoration: BoxDecoration(color: Colors.amber, borderRadius: BorderRadius.circular(6))),
                                        SizedBox(width: 6),
                                        Container(height: 12, width: 12, decoration: BoxDecoration(color: Colors.green, borderRadius: BorderRadius.circular(6))),
                                        Spacer(),
                                        Text('monoscript', style: TextStyle(color: Colors.white70, fontSize: 12)),
                                      ],
                                    ),
                                    Divider(color: Color(0xFF30363D), height: 24),
                                    _getHighlightedCode(selectedExample!.code)
                                  ],
                                ),
                              ),
                            )
                          : SizedBox(),
                      ),

                      SizedBox(width: 24), // Отступ между блоками
                      // Блок Features (справа)
                      Expanded(
                        flex: 3, // 30% места для Features
                        child: SizedBox(
                          height: 750, // Фиксированная высота равная блоку кода
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              // Консольный блок с фиксированной высотой
                              Container(
                                height: 260, // Фиксированная высота для консоли
                                padding: EdgeInsets.all(16),
                                decoration: BoxDecoration(
                                  color: Color(0xFF101010),
                                  borderRadius: BorderRadius.circular(8),
                                  border: Border.all(color: Color(0xFF333333), width: 1),
                                ),
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    // Заголовок консоли
                                    Row(
                                      children: [
                                        Icon(Icons.terminal, color: Colors.greenAccent, size: 18),
                                        SizedBox(width: 8),
                                        Text(
                                          'Output',
                                          style: TextStyle(
                                            fontSize: 18,
                                            fontWeight: FontWeight.bold,
                                            color: Colors.greenAccent,
                                            fontFamily: 'Fira Code',
                                          ),
                                        ),
                                        Spacer(),
                                        Container(
                                          padding: EdgeInsets.symmetric(horizontal: 6, vertical: 2),
                                          decoration: BoxDecoration(
                                            color: Color(0xFF333333),
                                            borderRadius: BorderRadius.circular(4),
                                          ),
                                          child: Text(
                                            'monoscript',
                                            style: TextStyle(
                                              fontSize: 12,
                                              color: Colors.white70,
                                              fontFamily: 'Fira Code',
                                            ),
                                          ),
                                        ),
                                      ],
                                    ),
                                    SizedBox(height: 12),
                                    // Консоль с фиксированной высотой и скроллингом
                                    Expanded(
                                      child: Container(
                                        decoration: BoxDecoration(
                                          color: Color(0xFF0A0A0A),
                                          borderRadius: BorderRadius.circular(4),
                                          border: Border.all(color: Color(0xFF2A2A2A)),
                                        ),
                                        child: SingleChildScrollView(
                                          child: Padding(
                                            padding: EdgeInsets.all(12),
                                            child: Column(
                                              crossAxisAlignment: CrossAxisAlignment.start,
                                              children: [
                                                Row(
                                                  children: [
                                                    Text(
                                                      '\$ ',
                                                      style: TextStyle(
                                                        color: Colors.greenAccent,
                                                        fontWeight: FontWeight.bold,
                                                        fontFamily: 'Fira Code',
                                                      ),
                                                    ),
                                                    Text(
                                                      'monoscript run example.mono',
                                                      style: TextStyle(
                                                        color: Colors.white,
                                                        fontFamily: 'Fira Code',
                                                      ),
                                                    ),
                                                  ],
                                                ),
                                                SizedBox(height: 8),
                                                Padding(
                                                  padding: EdgeInsets.only(left: 16),
                                                  child: _getConsoleOutput(selectedExample),
                                                )
                                              ],
                                            ),
                                          ),
                                        ),
                                      ),
                                    ),
                                    SizedBox(height: 12),

                                    // Информация о производительности
                                    Text(
                                      'Execution time: ${_getExecutionTime(selectedExample)} ms',
                                      style: TextStyle(
                                        fontSize: 12,
                                        color: Colors.grey,
                                        fontFamily: 'Fira Code',
                                      ),
                                    ),
                                  ],
                                ),
                              ),

                              SizedBox(height: 16),

                              // Блок Features с фиксированной высотой и скроллингом
                              Expanded(
                                child: Container(
                                  padding: EdgeInsets.fromLTRB(16.0, 16.0, 16.0, 16.0),
                                  decoration: BoxDecoration(
                                    color: Color(0xFF0D1321),
                                    border: Border.all(
                                      color: Color(0xFF1A2231),
                                      width: 1,
                                    ),
                                    borderRadius: BorderRadius.circular(8),
                                    boxShadow: [
                                      BoxShadow(
                                        color: Colors.black12,
                                        blurRadius: 8,
                                        offset: Offset(0, 4),
                                      ),
                                    ],
                                  ),
                                  child: SingleChildScrollView(
                                    child: _buildCodeFeatureSection(),
                                  ),
                                ),
                              ),
                            ],
                          ),
                        ),
                      ),
                      ],
                  ),
                ),
                
                // Дополнительное пространство внизу
                SizedBox(height: 40),
              ],
            ),
          ),
        ),
      ],
    ),
  );
}

Widget _smallOverview() {
  return Container(
    padding: EdgeInsets.all(24),
    margin: EdgeInsets.fromLTRB(80, 6, 48, 0),
    decoration: BoxDecoration(
      color: Color(0xFF0D1117).withOpacity(0.7),
      borderRadius: BorderRadius.circular(12),
      border: Border.all(
        color: Color(0xFF30363D),
        width: 1,
      ),
      boxShadow: [
        BoxShadow(
          color: Colors.black.withOpacity(0.2),
          blurRadius: 10,
          offset: Offset(0, 4),
        ),
      ],
    ),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          'MONOSCRIPT Language: Overview',
          style: TextStyle(
            fontSize: 24,
            fontWeight: FontWeight.bold,
            color: Colors.white,
          ),
        ),
        SizedBox(height: 16),
        
        // Main description
        Text(
          'MONOSCRIPT is a fast, intuitive programming language designed for secure code execution in various environments.',
          style: TextStyle(
            fontSize: 16,
            color: Colors.white,
            height: 1.5,
          ),
        ),
        SizedBox(height: 12),
        
        // Performance
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Icon(Icons.speed, color: Colors.orangeAccent, size: 24),
            SizedBox(width: 12),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    'High Performance',
                    style: TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                      color: Colors.white,
                    ),
                  ),
                  SizedBox(height: 4),
                  Text(
                    'Thanks to LLVM Backend, compiler written in C++ and standard library written in D and Zig, MONOSCRIPT provides optimal execution speed.',
                    style: TextStyle(
                      fontSize: 14,
                      color: Colors.white70,
                      height: 1.5,
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
        SizedBox(height: 16),
        
        // Security
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Icon(Icons.security, color: Colors.orangeAccent, size: 24),
            SizedBox(width: 12),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    'Secure Execution',
                    style: TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                      color: Colors.white,
                    ),
                  ),
                  SizedBox(height: 4),
                  Text(
                    'Created with an emphasis on code execution security in various environments, including Kubernetes and containers (more details in a separate section).',
                    style: TextStyle(
                      fontSize: 14,
                      color: Colors.white70,
                      height: 1.5,
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
        SizedBox(height: 24),
        
        // Supported paradigms
        Text(
          'Supported Paradigms',
          style: TextStyle(
            fontSize: 20,
            fontWeight: FontWeight.bold,
            color: Colors.white,
          ),
        ),
        SizedBox(height: 12),
        
        // Paradigms table
        Container(
          decoration: BoxDecoration(
            color: Color(0xFF1A2231),
            borderRadius: BorderRadius.circular(8),
            border: Border.all(color: Color(0xFF2A3142), width: 1),
          ),
          child: Column(
            children: [
              _paradigmRow('Imperative', 'Traditional constructs: if, for, while, mutability, and step-by-step execution', true),
              _paradigmRow('Functional', 'Lambdas, pure functions, pattern matching', false),
              _paradigmRow('Modular', 'Organization using use, modules and namespaces', true),
              _paradigmRow('Structural', 'Custom structures with methods and encapsulation support', false),
            ],
          ),
        ),
      ],
    ),
  );
}

Widget _footer() {
  return Container(
    width: double.infinity,
    padding: EdgeInsets.fromLTRB(80, 40, 80, 40),
    color: Color(0xFF0A0E19),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  crossAxisAlignment: CrossAxisAlignment.baseline,
                  textBaseline: TextBaseline.alphabetic,
                  children: [
                    Container(
                      padding: EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                      decoration: BoxDecoration(
                        color: Colors.white,
                        borderRadius: BorderRadius.circular(6),
                      ),
                      child: Text(
                        'MONO',
                        style: TextStyle(
                          fontSize: 22,
                          fontWeight: FontWeight.bold,
                          color: Colors.black,
                          height: 1.0,
                        ),
                      ),
                    ),
                    Text(
                      'script',
                      style: TextStyle(
                        fontSize: 18,
                        fontWeight: FontWeight.normal,
                        color: Colors.white,
                      ),
                    ),
                  ],
                ),
                SizedBox(height: 8),
                Text(
                  "Secure, readable plugin language",
                  style: TextStyle(
                    color: Colors.white70,
                    fontSize: 14,
                  ),
                ),
              ],
            ),
            Spacer(),
            // Footer Navigation
            Row(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                _footerNavSection(
                  'Documentation',
                  ['Language Reference', 'Standard Library', 'Security', 'Installation'],
                ),
                SizedBox(width: 64),
                _footerNavSection(
                  'Community',
                  ['GitHub', 'Discord', 'Twitter', 'Blog'],
                ),
                SizedBox(width: 64),
                _footerNavSection(
                  'Resources',
                  ['Tutorials', 'Examples', 'Use Cases', 'FAQ'],
                ),
              ],
            ),
          ],
        ),
        
        SizedBox(height: 40),
        Divider(color: Color(0xFF1A2231), thickness: 1),
        SizedBox(height: 24),
        
        Row(
          children: [
            Text(
              '© 2025 MONOPTICAL Corp. All rights reserved',
              style: TextStyle(
                color: Colors.white60,
                fontSize: 14,
              ),
            ),
            Spacer(),
            Row(
              children: [
                _socialIcon(Icons.alternate_email, 'Email'),
                SizedBox(width: 16),
                _socialIcon(Icons.discord, 'Discord'),
                SizedBox(width: 16),
                _socialIcon(Icons.cloud, 'GitHub'),
                SizedBox(width: 16),
                _socialIcon(Icons.language, 'Website'),
              ],
            ),
          ],
        ),
        
        SizedBox(height: 24),
        Row(
          children: [
            Text(
              'Made with security in mind',
              style: TextStyle(
                color: Colors.white38,
                fontSize: 12,
              ),
            ),
            SizedBox(width: 8),
            Icon(
              Icons.security,
              color: Colors.orangeAccent,
              size: 14,
            ),
            Spacer(),
            Text(
              'Version 0.1 Beta',
              style: TextStyle(
                color: Colors.orangeAccent,
                fontSize: 12,
                fontWeight: FontWeight.bold,
              ),
            ),
          ],
        ),
      ],
    ),
  );
}

Widget _footerNavSection(String title, List<String> links) {
  return Column(
    crossAxisAlignment: CrossAxisAlignment.start,
    children: [
      Text(
        title,
        style: TextStyle(
          color: Colors.white,
          fontSize: 18,
          fontWeight: FontWeight.bold,
        ),
      ),
      SizedBox(height: 16),
      ...links.map((link) => Padding(
        padding: EdgeInsets.only(bottom: 8),
        child: InkWell(
          onTap: () {},
          child: Text(
            link,
            style: TextStyle(
              color: Colors.white70,
              fontSize: 14,
            ),
          ),
        ),
      )),
    ],
  );
}

Widget _socialIcon(IconData icon, String tooltip) {
  return Tooltip(
    message: tooltip,
    child: InkWell(
      onTap: () {},
      child: Container(
        padding: EdgeInsets.all(8),
        decoration: BoxDecoration(
          color: Color(0xFF1A2231),
          borderRadius: BorderRadius.circular(8),
        ),
        child: Icon(
          icon,
          color: Colors.white70,
          size: 18,
        ),
      ),
    ),
  );
}

Widget _paradigmRow(String name, String description, bool isAlternate) {
  return Container(
    padding: EdgeInsets.symmetric(horizontal: 16, vertical: 12),
    decoration: BoxDecoration(
      color: isAlternate ? Color(0xFF21293B) : Colors.transparent,
      border: Border(
        bottom: BorderSide(
          color: Color(0xFF2A3142),
          width: 1,
        ),
      ),
    ),
    child: Row(
      children: [
        SizedBox(
          width: 150,
          child: Text(
            name,
            style: TextStyle(
              fontSize: 16,
              fontWeight: FontWeight.bold,
              color: Colors.orangeAccent,
            ),
          ),
        ),
        Expanded(
          child: Text(
            description,
            style: TextStyle(
              fontSize: 14,
              color: Colors.white70,
            ),
          ),
        ),
      ],
    ),
  );
}
  
Widget _buildTitle() {
  return Row(
    crossAxisAlignment: CrossAxisAlignment.baseline,
    textBaseline: TextBaseline.alphabetic,
    children: [
      Text(
        'The ',
        style: TextStyle(
          fontSize: 35,
          fontWeight: FontWeight.normal,
          color: Colors.white,
        ),
      ),
      Text(
        'MONO',
        style: TextStyle(
          fontSize: 35,
          fontWeight: FontWeight.bold,
          color: Colors.orangeAccent,
        ),
      ),
      Text(
        'script',
        style: TextStyle(
          fontSize: 25,
          fontWeight: FontWeight.normal,
          color: Colors.white,
        ),
      ),
      Text(
        ' Programming Language',
        style: TextStyle(
          fontSize: 35,
          fontWeight: FontWeight.bold,
          color: Colors.white,
        ),
      ),
      Transform.translate(
        offset: Offset(5, -20), // Смещение вверх
        child: Container(
          padding: EdgeInsets.symmetric(horizontal: 6, vertical: 2),
          decoration: BoxDecoration(
            color: Colors.orangeAccent,
            borderRadius: BorderRadius.circular(4),
          ),
          child: Text(
            '0.1 Beta',
            style: TextStyle(
              fontSize: 12, // Уменьшенный размер шрифта
              fontWeight: FontWeight.bold,
              color: Colors.black,
            ),
          ),
        ),
      ),
    ],
  );
}
  
  Widget _buildButtonRow() {
    return Row(
      children: <Widget>[
        ElevatedButton.icon(
          icon: Icon(Icons.code),
          label: Text('github.com/MONOPTICAL/MONOSCRIPT'),
          onPressed: () {
            js.context.callMethod('open', ['https://github.com/MONOPTICAL/MONOSCRIPT']);
          },
          style: ElevatedButton.styleFrom(
            backgroundColor: Color(0xFF252A34),
            foregroundColor: Colors.white,
            elevation: 2,
            padding: EdgeInsets.symmetric(horizontal: 20, vertical: 12),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(8),
            ),
          ),
        ),
        SizedBox(width: 16),
        ElevatedButton.icon(
          icon: Icon(Icons.download_for_offline),
          label: Text('Download for Linux'),
          onPressed: () {},
          style: ElevatedButton.styleFrom(
            backgroundColor: Color(0xFF17285A),
            foregroundColor: Colors.white,
            elevation: 2,
            padding: EdgeInsets.symmetric(horizontal: 20, vertical: 12),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(8),
            ),
          ),
        ),
      ],
    );
  }
  
  Widget _buildExampleChips() {
    return Wrap(
      spacing: 12,
      runSpacing: 12,
      children: examples.map((example) {
        return ChoiceChip(
          label: Text(example.name),
          selected: selectedExample == example,
          onSelected: (selected) {
            setState(() {
              selectedExample = example;
            });
          },
          selectedColor: Colors.orangeAccent,
          backgroundColor: Color(0xFF252525),
          padding: EdgeInsets.symmetric(horizontal: 12, vertical: 8),
          labelStyle: TextStyle(
            color: selectedExample == example 
                ? Colors.black 
                : Colors.white,
            fontWeight: selectedExample == example 
                ? FontWeight.bold 
                : FontWeight.normal,
          ),
        );
      }).toList(),
    );
  }
  
Widget _buildCodeFeatureSection() {
  List<Map<String, String>> features = [];
  
  if (selectedExample != null) {
    switch (selectedExample!.name) {
      case 'Fibonacci':
        features = [
          {
            'title': 'Recursion',
            'description': 'Function calls itself to calculate Fibonacci sequence'
          },
          {
            'title': 'Type Casting',
            'description': 'Explicit casting using the "->" operator (n - 1->i64)'
          },
          {
            'title': '@pure Annotation',
            'description': 'Functions marked as pure have no side effects'
          },
          {
            'title': '@strict Annotation',
            'description': 'Ensures stricter type checking and memory safety'
          },
        ];
        break;
      
      case 'Hello World':
        features = [
          {
            'title': 'Entry Point Declaration',
            'description': '@entry annotation marks the program\'s entry point'
          },
          {
            'title': 'void Return Type',
            'description': 'Function doesn\'t return any value'
          },
          {
            'title': 'Echo Statement',
            'description': 'Built-in echo function for console output'
          },
          {
            'title': 'String Literals',
            'description': 'Direct usage of string literals with double quotes'
          },
        ];
        break;
      
      case 'Sort':
        features = [
          {
            'title': 'Array Manipulation',
            'description': 'Creating and manipulating typed arrays (array<i32>)'
          },
          {
            'title': 'For-in Loop',
            'description': 'Iterating over ranges using for-in syntax'
          },
          {
            'title': 'Initialize-Assign Operator',
            'description': 'Uses ^= to initialize variables with their first value'
          },
          {
            'title': 'Array Indexing',
            'description': 'Direct access to array elements with [] notation'
          },
        ];
        break;
      
      case 'General Code':
        features = [
          {
            'title': 'String Concatenation',
            'description': 'Joining strings with "+" operator'
          },
          {
            'title': 'Boolean Operations',
            'description': 'Complex boolean conditions with comparison operators'
          },
          {
            'title': 'Type Conversion',
            'description': 'Converting between data types using toString_int()'
          },
          {
            'title': 'Multiple Functions',
            'description': 'Code organization with helper functions'
          },
        ];
      case 'Modules':
        features = [
          {
            'title': 'Module Declaration',
            'description': 'Using "use" keyword to import modules'
          },
          {
            'title': 'Final Variables',
            'description': 'Using "final" for constant values'
          },
          {
            'title': 'Module Testing',
            'description': 'Testing module functions in a separate file'
          },
          {
            'title': 'Entry Point Declaration',
            'description': '@entry annotation marks the program\'s entry point'
          },
        ];
        break;
    }
  }
  
  return Column(
    crossAxisAlignment: CrossAxisAlignment.start,
    children: [
      Text(
        'Code Features',
        style: TextStyle(
          fontSize: 22, 
          fontWeight: FontWeight.bold, 
          color: Colors.white
        ),
      ),
      SizedBox(height: 16),
      ...features.map((feature) => _featureItem(
        feature['title']!, 
        feature['description']!
      )),
    ],
  );
}
  
  Widget _featureItem(String title, String description) {
    return Container(
      margin: EdgeInsets.only(bottom: 16),
      padding: EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: Color(0xFF1A2231),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Color(0xFF2A3142), width: 1),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Icon(Icons.check_circle, color: Colors.orangeAccent, size: 20),
          SizedBox(width: 12),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  title,
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                  ),
                ),
                SizedBox(height: 4),
                Text(
                  description,
                  style: TextStyle(
                    fontSize: 14,
                    color: Colors.white70,
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

Widget _navBarItem(String title, {bool isActive = false}) {
  String firstPart = '';
  String secondPart = '';
  
  switch(title) {
    case 'Overview':
      firstPart = 'OVER';
      secondPart = 'view';
      break;
    case 'Get Started':
      firstPart = 'GET';
      secondPart = 'started';
      break;
    case 'Docs':
      firstPart = 'DOC';
      secondPart = 's';
      break;
    case 'Community':
      firstPart = 'COMM';
      secondPart = 'unity';
      break;
    case 'Blog':
      firstPart = 'BLOG';
      secondPart = '';
      break;
    default:
      firstPart = title;
      secondPart = '';
  }
  
  return Padding(
    padding: const EdgeInsets.symmetric(horizontal: 12.0),
    child: MouseRegion(
      cursor: SystemMouseCursors.click,
      child: AnimatedContainer(
        duration: Duration(milliseconds: 200),
        padding: EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
        decoration: BoxDecoration(
          border: Border(
            bottom: BorderSide(
              color: isActive ? Color(0xFF1A1A1A) : Colors.transparent,
              width: 2,
            ),
          ),
        ),
        child: InkWell(
          onTap: () {},
          splashColor: Colors.transparent,
          hoverColor: Colors.transparent,
          highlightColor: Colors.transparent,
          child: RichText(
            text: TextSpan(
              children: [
                TextSpan(
                  text: firstPart,
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                    color: isActive ? Color(0xFF1A1A1A) : Color(0xFF333333),
                    fontSize: 18,
                    height: 1.0,
                    letterSpacing: 0.5,
                  ),
                ),
                TextSpan(
                  text: secondPart,
                  style: TextStyle(
                    fontWeight: FontWeight.w300,
                    color: isActive ? Color(0xFF333333) : Color(0xFF666666),
                    fontSize: 14,
                    height: 1.0,
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    ),
  );
}

Widget _getConsoleOutput(CodeExample? example) {
  if (example == null) return Text('No example selected', style: TextStyle(color: Colors.red));
  
  switch (example.name) {
    case 'Fibonacci':
      return Text(
        '2971215073',
        style: TextStyle(
          color: Colors.white,
          fontFamily: 'Fira Code',
        ),
      );
    case 'Hello World':
      return Text(
        'Hello World!!!',
        style: TextStyle(
          color: Colors.white,
          fontFamily: 'Fira Code',
        ),
      );
    case 'Sort':
      return Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Before sort: [5, 3, 1, 4, 2]',
            style: TextStyle(
              color: Colors.grey,
              fontFamily: 'Fira Code',
            ),
          ),
          SizedBox(height: 4),
          Text(
            'After sort: [1, 2, 3, 4, 5]',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Fira Code',
            ),
          ),
        ],
      );
    case 'General Code':
      return Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Hellox hz2 World!',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Fira Code',
            ),
          ),
          SizedBox(height: 4),
          Text(
            '[main exited with code 4]',
            style: TextStyle(
              color: Colors.grey,
              fontFamily: 'Fira Code',
            ),
          ),
        ],
      );
    case 'Modules':
      return Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'a: 5, b: 10',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Fira Code',
            ),
          ),
          Text(
            'Sum: 15, Mul: 50',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Fira Code',
            ),
          ),
          SizedBox(height: 4),
          Text(
            '[moduleTest exited with code 0]',
            style: TextStyle(
              color: Colors.grey,
              fontFamily: 'Fira Code',
            ),
          ),
        ],
      );
    default:
      return Text(
        'Программа выполнена успешно.',
        style: TextStyle(
          color: Colors.white,
          fontFamily: 'Fira Code',
        ),
      );
  }
}

  String _getExecutionTime(CodeExample? example) {
    if (example == null) return '0';
    
    switch (example.name) {
      case 'Fibonacci':
        return '6572';
      case 'Hello World':
        return '0.73';
      case 'Sort':
        return '0.45';
      case 'General Code':
        return '0.83';
      case 'Modules':
        return '0.63';
      default:
        return '1.23';
    }
  }
}
