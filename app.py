# static and templete folder should be at same path as that of python files
# static folder file can be accesed by giving localhost\static\filename(when file name given that file will be downloaded)
# templetes folder : if we want to send html page to local host it can be done through templete
# bootstrap templetes => getbotstrap.com -> cop paste
from flask import Flask, render_template, request, redirect     # importing flask from Flask module , request => to get form values

                            # if we want to take a file from templete import class render_template
#importing SQLALCHEMY for database creation.
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime   #   to get date and time
app = Flask(__name__)
# configure the SQLite database, relative to the app instance folder
app.config['SQLALCHEMY_DATABASE_URI'] = "sqlite:///todo.db"
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False
db = SQLAlchemy(app)
# app.app_context.push()


#creating  a data base

class Todo(db.Model):   # creates a class Todo, => db.Model refers that it is an SQLAlchemy model
    sno = db.Column(db.Integer, primary_key = True)     # creates a column named sno, which stores integer value and it is primary key of the table
                                                        # primary key is unique identifier of a table
    title = db.Column(db.String(200), nullable = False) # creates a title column which stores string with maximum of 200 words, nullable = false means this cannot be blank

    desc = db.Column(db.String(500), nullable = False)  # creates a title column which stores string with maximum of 200 words, nullable = false means this cannot be blank
    date_created = db.Column(db.DateTime, default=datetime.utcnow)   # creates a column that stores date and tie, datetime.utcnow will give date and time at the moment column created

    def __repr__(self) -> str:
         return f"{self.sno} - {self.title}"

@app.route('/', methods=['GET','POST'])     # methods=['GET','POST'] => to receive value from post
def hello_world():
    if request.method=='POST':
        # print(request.form['title'])
        title = request.form['title']   # storing title in variable title
        desc = request.form['desc']     # storing desc in variable desc
        todo = Todo(title=title, desc=desc)
        db.session.add(todo)
        db.session.commit()
    allTodo = Todo.query.all()      # this will stole all the values in the TODO class
    # return 'Hello, Me!'      # app.run() will return hello, me in home page
    return render_template('index.html', allTodoJinja = allTodo)        

@app.route('/show')
def show():
    allTodo = Todo.query.all()
    print(allTodo)
    return 'when this page is opened all values in TODO class will be shown'      # app.run() will return hello, me in product page

@app.route('/update/<int:sno>', methods=['GET','POST'])
def update(sno):
    if request.method == 'POST':
        title = request.form['title']
        desc = request.form['desc']
        allTodo = Todo.query.filter_by(sno=sno).first()
        allTodo.title = title
        allTodo.desc = desc
        db.session.add(allTodo)
        db.session.commit()
        return redirect('/')
    allTodo = Todo.query.filter_by(sno=sno).first()
    return render_template('update.html', allTodoJinja = allTodo)      # app.run() will return hello, me in product page
                
@app.route('/delete/<int:sno>')     # sno takes serial num and send to delete method
def delete(sno):
    
    todo = Todo.query.filter_by(sno=sno).first()
    db.session.delete(todo)
    db.session.commit()
    return redirect("/")      # app.run() redirect to home page
        

if __name__ == "__main__":
    app.run(debug = True)       # debug = True => u can see the error in browser 