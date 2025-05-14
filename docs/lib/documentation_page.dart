import 'package:flutter/material.dart';
import 'package:syntax_highlight/syntax_highlight.dart';
import 'overview.dart';
import 'dart:js' as js;

class DocumentationPage extends StatefulWidget {
  @override
  _DocumentationPageState createState() => _DocumentationPageState();
}

class _DocumentationPageState extends State<DocumentationPage> {
  final TextEditingController _searchController = TextEditingController();
  String _searchQuery = '';
  final List<String> _recentDocs = [
    'Быстрый старт',
    'Типы данных',
    'Функции и аннотации',
    'Модули',
    'Безопасность'

  ];
  Highlighter? highlighter;
  String _currentLanguage = 'EN';

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
      return Text.rich(
        highlighter!.highlight(code),
        style: TextStyle(
          fontFamily: 'Fira Code',
          fontSize: 12,
          height: 1.5,
          color: Colors.white,
        ),
        );
    } else {
      return Text(
        code,
        style: TextStyle(
          fontFamily: 'Fira Code',
          fontSize: 12,
          height: 1.5,
          color: Colors.white,
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    if (highlighter == null) {
      _initializeHighlighter();
    }
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
                  _navBarItem('Overview'),
                  _navBarItem('Get Started'),
                  _navBarItem('Docs', isActive: true),
                  _navBarItem('Community'),
                  _navBarItem('Blog'),
                  Padding(
                    padding: const EdgeInsets.only(left: 24.0),
                    child: Row(
                      children: [
                        Container(
                          height: 24,
                          width: 1,
                          color: Color(0xFFCCCCCC),
                        ),
                        SizedBox(width: 24),
                        _languageSelector(),
                      ],
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
      body: SingleChildScrollView(
        child: Column(
          children: <Widget>[
            _headerSection(),
            _searchSection(),
            _mainDocsGridSection(),
            _recentDocsSection(),
            _versionsSection(),
            _footer(),
          ],
        ),
      ),
    );
  }

  Widget _headerSection() {
    return Container(
      padding: EdgeInsets.fromLTRB(80.0, 48.0, 80.0, 32.0),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Expanded(
            flex: 3,
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  'Документация',
                  style: TextStyle(
                    fontSize: 45,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                  ),
                ),
                SizedBox(height: 16),
                Text(
                  'Полное руководство по языку MONOSCRIPT: синтаксис, функции, библиотеки и лучшие практики',
                  style: TextStyle(
                    fontSize: 18,
                    color: Colors.white70,
                  ),
                ),
                SizedBox(height: 24),
                Row(
                  children: [
                    _categoryChip('Официальная документация', isActive: true),
                    SizedBox(width: 12),
                    _categoryChip('Примеры'),
                  ],
                ),
              ],
            ),
          ),
          SizedBox(width: 40),
          Container(
            width: MediaQuery.of(context).size.width * 0.2,
            child: Image(
              image: AssetImage('images/documentation_icon.png'),
              fit: BoxFit.contain,
            ),
          ),
        ],
      ),
    );
  }

  Widget _searchSection() {
    return Container(
      padding: EdgeInsets.symmetric(horizontal: 80, vertical: 32),
      decoration: BoxDecoration(
        color: Color(0xFF0D1117).withOpacity(0.5),
        border: Border(
          top: BorderSide(color: Color(0xFF30363D), width: 1),
          bottom: BorderSide(color: Color(0xFF30363D), width: 1),
        ),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Что вы хотите изучить?',
            style: TextStyle(
              fontSize: 24,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
          SizedBox(height: 16),
          Container(
            decoration: BoxDecoration(
              color: Color(0xFF1A2231),
              borderRadius: BorderRadius.circular(8),
              border: Border.all(color: Color(0xFF30363D), width: 1),
            ),
            padding: EdgeInsets.symmetric(horizontal: 16, vertical: 4),
            child: Row(
              children: [
                Icon(Icons.search, color: Colors.white70),
                SizedBox(width: 12),
                Expanded(
                  child: TextField(
                    controller: _searchController,
                    onChanged: (value) {
                      setState(() {
                        _searchQuery = value;
                      });
                    },
                    style: TextStyle(color: Colors.white),
                    decoration: InputDecoration(
                      hintText: 'Искать в документации...',
                      hintStyle: TextStyle(color: Colors.white70),
                      border: InputBorder.none,
                    ),
                  ),
                ),
                if (_searchQuery.isNotEmpty)
                  IconButton(
                    icon: Icon(Icons.clear, color: Colors.white70),
                    onPressed: () {
                      setState(() {
                        _searchController.clear();
                        _searchQuery = '';
                      });
                    },
                  ),
              ],
            ),
          ),
          SizedBox(height: 12),
          Row(
            children: [
              Text(
                'Популярные теги:',
                style: TextStyle(
                  color: Colors.white70,
                  fontSize: 14,
                ),
              ),
              SizedBox(width: 12),
              _tagChip('функции'),
              SizedBox(width: 8),
              _tagChip('безопасность'),
              SizedBox(width: 8),
              _tagChip('типы данных'),
              SizedBox(width: 8),
              _tagChip('модули'),
            ],
          ),
        ],
      ),
    );
  }

Widget _mainDocsGridSection() {
    return Container(
      padding: EdgeInsets.symmetric(horizontal: 80, vertical: 40),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Основные разделы',
            style: TextStyle(
              fontSize: 28,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
          SizedBox(height: 24),
          GridView.count(
            shrinkWrap: true,
            physics: NeverScrollableScrollPhysics(),
            crossAxisCount: 3,
            crossAxisSpacing: 24,
            mainAxisSpacing: 24,
            childAspectRatio: 0.9, // 1.2
            children: [
              _docCard(
                'Начало работы',
                'Установка, первая программа и основные концепции языка',
                Icons.play_circle,
                Color(0xFF2E5C87),
                codeExample: '[void]hello_world() @entry\n|   echo("Hello World!!!")',
              ),
              _docCard(
                'Руководство по синтаксису',
                'Подробное объяснение синтаксиса и структур языка',
                Icons.code,
                Color(0xFF1E4422),
                codeExample: """const string global_str = " "
[i32]main()
|   [string]hello_str() @strict
|   |   [string]getSpace() @strict
|   |   |   return global_str
|   |   const string space = getSpace()
|   |   return "Hello" + space + "World" + "!!!"
|   if (1 == 1)
|   |   echo(hello_str())
|   else
|   |   echo("This will never be printed")
|   return 0""",
              ),
              _docCard(
                'Типы данных',
                'Примитивные и составные типы, преобразования и проверки',
                Icons.data_object,
                Color(0xFF652E60),
                codeExample: 'i32 value = 42\nfloat pi = 3.14159\nstring text = "Example"',
              ),
              _docCard(
                'Функции',
                'Объявление, параметры, возвращаемые значения и аннотации',
                Icons.functions,
                Color(0xFF5E3A1E),
                codeExample: """[i64]fibonacci(i64: n) @strict @pure
|   if (n == 0)
|   |   return 0
|   else if (n == 1)
|   |   return 1
|   return fibonacci(n - 1->i64) + fibonacci(n - 2->i64)

[i32]main()
|   echo(toString_long(fibonacci(47))) // 2971215073
|   return 0""",
              ),
              _docCard(
                'Модули и библиотеки',
                'Организация кода в модули и использование библиотек',
                Icons.library_books,
                Color(0xFF17285A),
                codeExample: 'use\n|>  math\n|>  io\n|>  network',
              ),
              _docCard(
                'Безопасность',
                'Лучшие практики по обеспечению безопасности кода',
                Icons.security,
                Color(0xFF5A1717),
                codeExample: """[i64]endless_recursion(i64: i)
|   if (i == 0)
|   |   return 0
|   else
|   |   return 1 + endless_recursion(i + 1)
|   return 0 // unreachable

// Compiler detected Intentional stack overflow and rejected code execution
[i32]main()
|   echo(toString_long(endless_recursion(1))) // Intentional stack overflow
|   return 0""",
              ),
            ],
          ),
        ],
      ),
    );
  }

  Widget _recentDocsSection() {
    return Container(
      padding: EdgeInsets.symmetric(horizontal: 80, vertical: 24),
      decoration: BoxDecoration(
        color: Color(0xFF0D1117).withOpacity(0.5),
        border: Border(
          top: BorderSide(color: Color(0xFF30363D), width: 1),
          bottom: BorderSide(color: Color(0xFF30363D), width: 1),
        ),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              Icon(Icons.history, color: Colors.white),
              SizedBox(width: 12),
              Text(
                'Недавно просмотренные',
                style: TextStyle(
                  fontSize: 22,
                  fontWeight: FontWeight.bold,
                  color: Colors.white,
                ),
              ),
              Spacer(),
              TextButton(
                onPressed: () {},
                child: Text(
                  'Очистить историю',
                  style: TextStyle(color: Colors.white70),
                ),
              ),
            ],
          ),
          SizedBox(height: 16),
          Wrap(
            spacing: 16,
            runSpacing: 16,
            children: _recentDocs
                .map((doc) => _recentDocItem(doc))
                .toList(),
          ),
        ],
      ),
    );
  }

  Widget _versionsSection() {
    return Container(
      padding: EdgeInsets.symmetric(horizontal: 80, vertical: 40),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Документация по версиям',
            style: TextStyle(
              fontSize: 24,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
          SizedBox(height: 16),
          Container(
            decoration: BoxDecoration(
              color: Color(0xFF1A2231),
              borderRadius: BorderRadius.circular(8),
              border: Border.all(color: Color(0xFF30363D), width: 1),
            ),
            child: Column(
              children: [
                Container(
                  padding: EdgeInsets.all(16),
                  decoration: BoxDecoration(
                    border: Border(
                      bottom: BorderSide(color: Color(0xFF30363D)),
                    ),
                  ),
                  child: Row(
                    children: [
                      Text(
                        'Версия',
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.bold,
                          color: Colors.white70,
                        ),
                      ),
                      Spacer(),
                      Text(
                        'Дата выпуска',
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.bold,
                          color: Colors.white70,
                        ),
                      ),
                      SizedBox(width: 80),
                      Text(
                        'Статус',
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.bold,
                          color: Colors.white70,
                        ),
                      ),
                    ],
                  ),
                ),
                _versionItem('0.1.0 Beta', '10 мая 2025', 'Текущая', true),
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
                        child: Row(
                          crossAxisAlignment: CrossAxisAlignment.baseline,
                          textBaseline: TextBaseline.alphabetic,
                          children: [
                            Text(
                              'MONO',
                              style: TextStyle(
                                fontSize: 22,
                                fontWeight: FontWeight.bold,
                                color: Colors.black,
                                height: 1.0,
                              ),
                            ),
                            Text(
                              'script',
                              style: TextStyle(
                                fontSize: 18,
                                fontWeight: FontWeight.normal,
                                color: Colors.black,
                              ),
                            ),
                          ],
                        ),
                      )
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
              Row(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  _footerNavSection(
                    'Документация',
                    ['Справочник языка', 'Стандартная библиотека', 'API'],
                  ),
                  SizedBox(width: 64),
                  _footerNavSection(
                    'Сообщество',
                    ['GitHub', 'Discord', 'Twitter', 'Blog'],
                  ),
                  SizedBox(width: 64),
                  _footerNavSection(
                    'Ресурсы',
                    ['Примеры кода', 'Часто задаваемые вопросы', 'Поддержка'],
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
                color: Colors.white,
                size: 14,
              ),
              Spacer(),
              Text(
                'Version 0.1 Beta',
                style: TextStyle(
                  color: Colors.white,
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

Widget _docCard(String title, String description, IconData icon, Color color, {String? codeExample}) {
    return InkWell(
      onTap: () {},
      child: Container(
        padding: EdgeInsets.all(24),
        decoration: BoxDecoration(
          color: color.withOpacity(0.2),
          borderRadius: BorderRadius.circular(8),
          border: Border.all(color: color.withOpacity(0.5), width: 1),
        ),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            // Заголовок
            Row(
              children: [
                Icon(
                  icon,
                  color: Colors.white,
                  size: 28,
                ),
                SizedBox(width: 16),
                Expanded(
                  child: Text(
                    title,
                    style: TextStyle(
                      fontSize: 20,
                      fontWeight: FontWeight.bold,
                      color: Colors.white,
                    ),
                  ),
                ),
              ],
            ),
            
            SizedBox(height: 16),
            
            // Код в центре
            if (codeExample != null)
              Expanded(
                child: Container(
                  width: double.infinity,
                  margin: EdgeInsets.symmetric(vertical: 8),
                  decoration: BoxDecoration(
                    color: Color(0xFF0D1117),
                    borderRadius: BorderRadius.circular(6),
                    border: Border.all(color: Color(0xFF30363D)),
                    boxShadow: [
                      BoxShadow(
                        color: Colors.black.withOpacity(0.3),
                        blurRadius: 4,
                        offset: Offset(0, 2),
                      ),
                    ],
                  ),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      // Имитация строки с кнопками окна редактора кода
                      Container(
                        padding: EdgeInsets.symmetric(horizontal: 12, vertical: 6),
                        decoration: BoxDecoration(
                          color: Color(0xFF161B22),
                          borderRadius: BorderRadius.only(
                            topLeft: Radius.circular(5), 
                            topRight: Radius.circular(5)
                          ),
                          border: Border(
                            bottom: BorderSide(color: Color(0xFF30363D), width: 1),
                          ),
                        ),
                        child: Row(
                          mainAxisSize: MainAxisSize.min,
                          children: [
                            Container(height: 10, width: 10, decoration: BoxDecoration(
                              color: Colors.red.withOpacity(0.8), 
                              borderRadius: BorderRadius.circular(5)
                            )),
                            SizedBox(width: 6),
                            Container(height: 10, width: 10, decoration: BoxDecoration(
                              color: Colors.amber.withOpacity(0.8), 
                              borderRadius: BorderRadius.circular(5)
                            )),
                            SizedBox(width: 6),
                            Container(height: 10, width: 10, decoration: BoxDecoration(
                              color: Colors.green.withOpacity(0.8), 
                              borderRadius: BorderRadius.circular(5)
                            )),
                            Spacer(),
                            Text('monoscript', style: TextStyle(
                              color: Colors.white54, 
                              fontSize: 10, 
                              fontFamily: 'Fira Code'
                            )),
                          ],
                        ),
                      ),
                      // Код с подсветкой без ограничения высоты и прокрутки
                      Expanded(
                        child: Padding(
                          padding: EdgeInsets.all(12),
                          child: _getHighlightedCode(codeExample),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            
            SizedBox(height: 16),
            
            // Описание внизу
            Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  description,
                  style: TextStyle(
                    fontSize: 14,
                    color: Colors.white70,
                  ),
                ),
                
                SizedBox(height: 16),
                
                // Стрелка "подробнее"
                Align(
                  alignment: Alignment.bottomRight,
                  child: Icon(
                    Icons.arrow_forward,
                    color: Colors.white54,
                    size: 20,
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _recentDocItem(String title) {
    return InkWell(
      onTap: () {},
      child: Container(
        padding: EdgeInsets.symmetric(horizontal: 16, vertical: 12),
        decoration: BoxDecoration(
          color: Color(0xFF1A2231),
          borderRadius: BorderRadius.circular(8),
          border: Border.all(color: Color(0xFF30363D), width: 1),
        ),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            Icon(Icons.description, color: Colors.white70, size: 16),
            SizedBox(width: 8),
            Text(
              title,
              style: TextStyle(
                color: Colors.white,
                fontSize: 14,
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _versionItem(String version, String date, String status, [bool isCurrent = false]) {
    return Container(
      padding: EdgeInsets.symmetric(vertical: 12, horizontal: 16),
      decoration: BoxDecoration(
        color: isCurrent ? Color(0xFF1E4422).withOpacity(0.2) : Colors.transparent,
        border: Border(
          bottom: BorderSide(color: Color(0xFF30363D)),
        ),
      ),
      child: Row(
        children: [
          Text(
            version,
            style: TextStyle(
              fontWeight: FontWeight.bold,
              fontSize: 16,
              color: isCurrent ? Colors.white : Colors.white70,
            ),
          ),
          Spacer(),
          Text(
            date,
            style: TextStyle(
              fontSize: 16,
              color: Colors.white70,
            ),
          ),
          SizedBox(width: 80),
          Container(
            padding: EdgeInsets.symmetric(horizontal: 12, vertical: 4),
            decoration: BoxDecoration(
              color: isCurrent ? Color(0xFF1E4422) : Color(0xFF30363D),
              borderRadius: BorderRadius.circular(16),
            ),
            child: Text(
              status,
              style: TextStyle(
                fontSize: 14,
                color: isCurrent ? Colors.greenAccent : Colors.white70,
                fontWeight: isCurrent ? FontWeight.bold : FontWeight.normal,
              ),
            ),
          ),
          if (isCurrent)
            Padding(
              padding: const EdgeInsets.only(left: 8.0),
              child: Icon(Icons.check_circle, color: Colors.greenAccent, size: 16),
            ),
        ],
      ),
    );
  }

  Widget _categoryChip(String label, {bool isActive = false}) {
    return ChoiceChip(
      label: Text(label),
      checkmarkColor: Colors.black,
      selected: isActive,
      onSelected: (selected) {},
      selectedColor: Colors.white,
      backgroundColor: Color(0xFF1A2231),
      labelStyle: TextStyle(
        color: isActive ? Colors.black : Colors.white70,
        fontWeight: isActive ? FontWeight.bold : FontWeight.normal,
      ),
      padding: EdgeInsets.symmetric(horizontal: 12, vertical: 8),
    );
  }

  Widget _tagChip(String label) {
    return InkWell(
      onTap: () {},
      child: Container(
        padding: EdgeInsets.symmetric(horizontal: 12, vertical: 4),
        decoration: BoxDecoration(
          color: Color(0xFF2A3142),
          borderRadius: BorderRadius.circular(16),
        ),
        child: Text(
          '#$label',
          style: TextStyle(
            color: Colors.white70,
            fontSize: 12,
          ),
        ),
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
            onTap: () {
              switch (title) {
                case 'Overview':
                  Navigator.push(
                    context,
                    MaterialPageRoute(builder: (context) => MonoScriptPage()),
                  );
                  break;
                case 'Get Started':
                  break;
                case 'Docs':
                  break;
                case 'Community':
                  break;
                case 'Blog':
                  break;
              }
            },
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

    // Виджет переключателя языка
  Widget _languageSelector() {
    return Container(
      decoration: BoxDecoration(
        color: Color(0xFFF2F2F2),
        borderRadius: BorderRadius.circular(4),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          // Английский язык
          InkWell(
            onTap: () {
              setState(() {
                _currentLanguage = 'EN';
              });
            },
            child: Container(
              padding: EdgeInsets.symmetric(horizontal: 10, vertical: 6),
              decoration: BoxDecoration(
                color: _currentLanguage == 'EN' ? Color(0xFF333333) : Colors.transparent,
                borderRadius: BorderRadius.horizontal(left: Radius.circular(4)),
              ),
              child: Text(
                'EN',
                style: TextStyle(
                  color: _currentLanguage == 'EN' ? Colors.white : Color(0xFF333333),
                  fontWeight: FontWeight.bold,
                  fontSize: 12,
                ),
              ),
            ),
          ),
          // Русский язык
          InkWell(
            onTap: () {
              setState(() {
                _currentLanguage = 'RU';
              });
            },
            child: Container(
              padding: EdgeInsets.symmetric(horizontal: 10, vertical: 6),
              decoration: BoxDecoration(
                color: _currentLanguage == 'RU' ? Color(0xFF333333) : Colors.transparent,
                borderRadius: BorderRadius.horizontal(right: Radius.circular(4)),
              ),
              child: Text(
                'RU',
                style: TextStyle(
                  color: _currentLanguage == 'RU' ? Colors.white : Color(0xFF333333),
                  fontWeight: FontWeight.bold,
                  fontSize: 12,
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }

}

